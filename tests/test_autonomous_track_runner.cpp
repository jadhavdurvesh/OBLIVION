#include "oblivion/simulation/autonomous_track_runner.hpp"
#include <cmath>
#include <stdexcept>
namespace {void require(bool c,const char*m){if(!c)throw std::runtime_error(m);}}
void test_autonomous_track_runner(){
 using namespace oblivion;
 TrackBuilder builder(0.012);require(builder.add_straight({0,0},{1.0,0}),"runner track build failed");
 TrackRunnerConfig c{};
 c.plant.maximum_wheel_speed_mps=0.7;c.plant.maximum_wheel_acceleration_mps2=4.0;
 c.plant.motor_time_constant_seconds=0.015;c.motion.maximum_speed_mps=0.55;
 c.motion.maximum_wheel_speed_mps=0.7;c.motion.maximum_longitudinal_acceleration_mps2=1.0;
 c.motion.maximum_braking_deceleration_mps2=1.2;c.motion.maximum_lateral_acceleration_mps2=1.5;
 c.motion.maximum_yaw_rate_radps=5.0;c.motion.wheel_track_m=c.plant.track_width_m;
 c.simulated_sensors.sensor_count=5;c.perception.sensor_count=5;
 for(std::size_t i=0;i<5;++i){
  const double lateral=(static_cast<double>(i)-2.0)*0.01;
  c.simulated_sensors.lateral_positions_m[i]=lateral;
  c.perception.positions_m[i]=lateral;
  c.perception.dark_level[i]=0.0;c.perception.light_level[i]=1.0;
 }
 c.perception.line_is_dark=true;c.perception.minimum_total_strength=0.5;
 c.steering_gains={8.0,0.0,0.05};c.steering_limits={-0.5,0.5,-0.1,0.1};
 c.sensor_forward_offset_m=0.025;c.completion_radius_m=0.045;
 c.maximum_run_time_seconds=8.0;
 AutonomousTrackRunner runner(c);require(runner.configuration_valid(),"runner config invalid");
 const auto result=runner.run(builder.build(),{0,0,0},0.005,1600);
 require(result.valid,"closed-loop track run invalid");
 require(result.planned_motion.valid,"runner did not generate trajectory");
 require(result.steps>0,"runner did not execute simulation steps");
 require(result.maximum_cross_track_error_m<0.02,"straight-track tracking error too high");
 require(result.lost_line_steps==0,"line lost on simple straight track");
 require(result.completed,"robot did not complete straight track");
 require(result.final_pose.x_m>0.90,"robot did not reach track endpoint");
}
