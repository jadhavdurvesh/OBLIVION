#include "oblivion/simulation/autonomous_track_runner.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace oblivion {
AutonomousTrackRunner::AutonomousTrackRunner(TrackRunnerConfig c)noexcept
 :config_(c),plant_(c.plant),sensor_simulator_(c.simulated_sensors),
  perception_(c.perception),planner_(c.motion),steering_(c.steering_gains,c.steering_limits),
  motor_limiter_(c.motor_command_limit){}
bool AutonomousTrackRunner::configuration_valid()const noexcept{
 return plant_.configuration_valid()&&sensor_simulator_.configuration_valid()&&
 perception_.configuration_valid()&&planner_.constraints_valid()&&motor_limiter_.configuration_valid()&&
 config_.simulated_sensors.sensor_count==config_.perception.sensor_count&&
 config_.motor_command_limit<=1.0&&
 config_.motion.maximum_wheel_speed_mps<=config_.plant.maximum_wheel_speed_mps&&
 std::abs(config_.motion.wheel_track_m-config_.plant.track_width_m)<=1e-6&&
 std::isfinite(config_.sensor_forward_offset_m)&&config_.sensor_forward_offset_m>=0&&
 std::isfinite(config_.minimum_tracking_confidence)&&config_.minimum_tracking_confidence>=0&&
 config_.minimum_tracking_confidence<=1&&std::isfinite(config_.completion_radius_m)&&
 config_.completion_radius_m>0&&std::isfinite(config_.maximum_run_time_seconds)&&
 config_.maximum_run_time_seconds>0;
}
TrackRunMetrics AutonomousTrackRunner::run(const Track&t,const Pose2D&initial,double dt,std::size_t max_steps)noexcept{
 TrackRunMetrics metrics{};
 if(!configuration_valid()||!t.valid()||!std::isfinite(dt)||dt<=0||max_steps==0||
    !std::isfinite(initial.x_m)||!std::isfinite(initial.y_m)||!std::isfinite(initial.heading_rad))return metrics;
 std::array<PathSample,kMaxTrajectoryPoints> path{};
 std::array<double,kMaxTrajectoryPoints> cumulative{};
 if(t.point_count<2||t.point_count>t.centerline.size())return metrics;
 for(std::size_t i=1;i<t.point_count;++i){
  const double dx=t.centerline[i].x_m-t.centerline[i-1].x_m;
  const double dy=t.centerline[i].y_m-t.centerline[i-1].y_m;
  cumulative[i]=cumulative[i-1]+std::hypot(dx,dy);
 }
 const double total_length=cumulative[t.point_count-1];
 if(!std::isfinite(total_length)||total_length<1e-6)return metrics;
 const std::size_t count=std::min(kMaxTrajectoryPoints,
     std::max<std::size_t>(2,static_cast<std::size_t>(std::ceil(total_length/0.02))+1));
 for(std::size_t sample=0;sample<count;++sample){
  const double distance=total_length*static_cast<double>(sample)/static_cast<double>(count-1);
  std::size_t segment=1;
  while(segment<t.point_count-1&&cumulative[segment]<distance)++segment;
  const double segment_length=cumulative[segment]-cumulative[segment-1];
  const double fraction=segment_length>1e-12?(distance-cumulative[segment-1])/segment_length:0.0;
  const auto&a=t.centerline[segment-1];const auto&b=t.centerline[segment];
  path[sample]={{a.x_m+fraction*(b.x_m-a.x_m),a.y_m+fraction*(b.y_m-a.y_m)},0.0,0.0};
 }
 metrics.planned_motion=planner_.plan(path.data(),count,0.0);
 if(!metrics.planned_motion.valid)return metrics;
 plant_.reset(initial);steering_.reset();
 std::array<double,kMaxReflectanceSensors> raw{};
 double squared_error=0.0,sum_error=0.0;
 const double maximum_time=std::min(config_.maximum_run_time_seconds,dt*static_cast<double>(max_steps));
 const double max_speed=config_.plant.maximum_wheel_speed_mps;
 if(max_speed<=0)return metrics;
 for(std::size_t step=0;step<max_steps&&metrics.simulated_time_seconds<maximum_time;++step){
  const Pose2D pose=plant_.pose();
  const double forward_x=std::cos(pose.heading_rad)*config_.sensor_forward_offset_m;
  const double forward_y=std::sin(pose.heading_rad)*config_.sensor_forward_offset_m;
  const Pose2D sensor_pose{pose.x_m+forward_x,pose.y_m+forward_y,pose.heading_rad};
  if(!sensor_simulator_.sample(t,sensor_pose,static_cast<std::uint64_t>(metrics.simulated_time_seconds*1e6),
                               raw.data(),raw.size()))return metrics;
  const auto observation=perception_.process(raw.data(),config_.perception.sensor_count,
                  static_cast<std::uint64_t>(metrics.simulated_time_seconds*1e6));
  if(!observation.input_valid){
   ++metrics.sensor_fault_steps;
   metrics.faulted=true;
   const MotorCommand stop=motor_limiter_.apply({0.0,0.0});
   if(!plant_.step(stop,dt))return metrics;
   metrics.simulated_time_seconds+=dt;
   metrics.steps=step+1;
   break;
  }
  if(!observation.detected){
   ++metrics.lost_line_steps;
   // Fail closed: no line means no further acceleration.
   const MotorCommand braking=motor_limiter_.apply({0.0,0.0});
   if(!plant_.step(braking,dt))return metrics;
   metrics.simulated_time_seconds+=dt;metrics.steps=step+1;
   continue;
  }
  if(observation.confidence<config_.minimum_tracking_confidence)++metrics.low_confidence_steps;
  const double error=observation.position_m;
  const double abs_error=std::abs(error);
  metrics.maximum_cross_track_error_m=std::max(metrics.maximum_cross_track_error_m,abs_error);
  squared_error+=error*error;sum_error+=abs_error;
  // Find the nearest point in the precomputed speed profile. This is a
  // bounded O(N) search over at most 512 points and is deterministic.
  std::size_t nearest=0;double nearest_d2=std::numeric_limits<double>::infinity();
  for(std::size_t i=0;i<metrics.planned_motion.count;++i){
   const auto&p=metrics.planned_motion.points[i].position;
   const double dx=p.x_m-pose.x_m,dy=p.y_m-pose.y_m,d2=dx*dx+dy*dy;
   if(d2<nearest_d2){nearest_d2=d2;nearest=i;}
  }
  const auto& target=metrics.planned_motion.points[nearest];
  const double base_left=target.left_wheel_speed_mps/max_speed;
  const double base_right=target.right_wheel_speed_mps/max_speed;
  const auto control=steering_.update({error,0.0,dt});
  if(!control.valid)return metrics;
  // Positive sensor error means the line is left of the robot: slow left,
  // speed right to create positive (left) yaw.
  const MotorCommand requested{base_left-control.command,base_right+control.command};
  const MotorCommand safe=motor_limiter_.apply(requested);
  if(!plant_.step(safe,dt))return metrics;
  metrics.simulated_time_seconds+=dt;metrics.steps=step+1;
  const auto after=plant_.pose();
  const double remaining=std::hypot(t.centerline[t.point_count-1].x_m-after.x_m,
                                    t.centerline[t.point_count-1].y_m-after.y_m);
  if(remaining<=config_.completion_radius_m){metrics.completed=true;break;}
 }
 metrics.final_pose=plant_.pose();
 metrics.timed_out=!metrics.completed&&metrics.simulated_time_seconds>=maximum_time;
 const std::size_t observed_steps=metrics.steps-metrics.lost_line_steps;
 metrics.mean_cross_track_error_m=observed_steps?sum_error/static_cast<double>(observed_steps):0.0;
 metrics.rms_cross_track_error_m=observed_steps?std::sqrt(squared_error/static_cast<double>(observed_steps)):0.0;
 metrics.estimated_time_seconds=metrics.planned_motion.estimated_time_seconds;
 metrics.valid=metrics.steps>0&&std::isfinite(metrics.final_pose.x_m)&&
               std::isfinite(metrics.rms_cross_track_error_m);
 return metrics;
}
} // namespace oblivion
