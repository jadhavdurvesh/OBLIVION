#include "oblivion/motion/planner.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace oblivion {
namespace { constexpr double kEpsilon=1e-10; }
MotionPlanner::MotionPlanner(MotionConstraints c)noexcept:constraints_(c){}
bool MotionPlanner::constraints_valid()const noexcept{
 const auto&c=constraints_;
 return std::isfinite(c.maximum_speed_mps)&&std::isfinite(c.maximum_wheel_speed_mps)&&
 std::isfinite(c.maximum_lateral_acceleration_mps2)&&std::isfinite(c.maximum_longitudinal_acceleration_mps2)&&
 std::isfinite(c.maximum_braking_deceleration_mps2)&&std::isfinite(c.maximum_yaw_rate_radps)&&
 std::isfinite(c.wheel_track_m)&&std::isfinite(c.gravity_mps2)&&
 std::isfinite(c.planning_latency_seconds)&&std::isfinite(c.safety_margin_m)&&
 c.maximum_speed_mps>0&&c.maximum_wheel_speed_mps>0&&c.maximum_lateral_acceleration_mps2>0&&
 c.maximum_longitudinal_acceleration_mps2>0&&c.maximum_braking_deceleration_mps2>0&&
 c.maximum_yaw_rate_radps>0&&c.wheel_track_m>0&&c.gravity_mps2>0&&
 c.planning_latency_seconds>=0&&c.safety_margin_m>=0;
}
double MotionPlanner::curvature(MotionPoint2D a,MotionPoint2D b,MotionPoint2D c)const noexcept{
 const double ab=std::hypot(b.x_m-a.x_m,b.y_m-a.y_m);
 const double bc=std::hypot(c.x_m-b.x_m,c.y_m-b.y_m);
 const double ca=std::hypot(a.x_m-c.x_m,a.y_m-c.y_m);
 const double denominator=ab*bc*ca;
 if(!std::isfinite(denominator)||denominator<kEpsilon)return 0.0;
 const double cross=(b.x_m-a.x_m)*(c.y_m-a.y_m)-(b.y_m-a.y_m)*(c.x_m-a.x_m);
 return 2.0*cross/denominator;
}
double MotionPlanner::maximum_corner_speed(double k,double grade,double bank)const noexcept{
 if(!constraints_valid()||!std::isfinite(k)||!std::isfinite(grade)||!std::isfinite(bank))return 0.0;
 const double abs_k=std::abs(k);
 if(abs_k<kEpsilon)return constraints_.maximum_speed_mps;
 const double effective_lateral=std::max(0.0,constraints_.maximum_lateral_acceleration_mps2+
      std::copysign(constraints_.gravity_mps2*std::sin(bank)*std::cos(grade),k));
 const double lateral=std::sqrt(effective_lateral/abs_k);
 const double yaw=constraints_.maximum_yaw_rate_radps/abs_k;
 const double wheel=constraints_.maximum_wheel_speed_mps/
      (1.0+0.5*constraints_.wheel_track_m*abs_k);
 return std::min({constraints_.maximum_speed_mps,lateral,yaw,wheel});
}
double MotionPlanner::turn_anticipation_distance(double speed,double decel)const noexcept{
 if(!constraints_valid()||!std::isfinite(speed)||!std::isfinite(decel)||speed<0||decel<=0)
   return std::numeric_limits<double>::infinity();
 return speed*constraints_.planning_latency_seconds+
        speed*speed/(2.0*decel)+constraints_.safety_margin_m;
}
WheelSpeeds MotionPlanner::wheel_speeds(double v,double k)const noexcept{
 if(!constraints_valid()||!std::isfinite(v)||!std::isfinite(k))return {};
 const double omega=v*k,half=0.5*constraints_.wheel_track_m*omega;
 return {v-half,v+half};
}
MotionPlan MotionPlanner::plan(const PathSample* path,std::size_t count,double terminal)const noexcept{
 MotionPlan result{};
 if(!constraints_valid()||path==nullptr||count<2||count>kMaxTrajectoryPoints||
    !std::isfinite(terminal)||terminal<0||terminal>constraints_.maximum_speed_mps)return result;
 std::array<double,kMaxTrajectoryPoints> ds{},curv{},speed{};
 for(std::size_t i=0;i<count;++i){
  const auto&p=path[i];
  if(!std::isfinite(p.position.x_m)||!std::isfinite(p.position.y_m)||!std::isfinite(p.grade_rad)||!std::isfinite(p.bank_rad))return result;
  if(i>0){ds[i]=std::hypot(p.position.x_m-path[i-1].position.x_m,p.position.y_m-path[i-1].position.y_m);
   if(ds[i]<kEpsilon)return result;}
  if(i>0&&i+1<count)curv[i]=curvature(path[i-1].position,p.position,path[i+1].position);
  const double local=maximum_corner_speed(curv[i],p.grade_rad,p.bank_rad);
  if(!std::isfinite(local)||local<=0)return result;
  speed[i]=local;
 }
 speed[count-1]=std::min(speed[count-1],terminal);
 // Backward pass: braking feasibility, including grade contribution.
 for(std::size_t i=count-1;i>0;--i){
  const double grade=path[i].grade_rad;
  const double available=std::max(0.05,constraints_.maximum_braking_deceleration_mps2+
                                      constraints_.gravity_mps2*std::sin(grade));
  const double reachable=std::sqrt(std::max(0.0,speed[i]*speed[i]+2.0*available*ds[i]));
  speed[i-1]=std::min(speed[i-1],reachable);
 }
 // Forward pass: acceleration feasibility.
 for(std::size_t i=1;i<count;++i){
  const double available=std::max(0.05,constraints_.maximum_longitudinal_acceleration_mps2-
      constraints_.gravity_mps2*std::sin(path[i].grade_rad));
  const double reachable=std::sqrt(std::max(0.0,speed[i-1]*speed[i-1]+2.0*available*ds[i]));
  speed[i]=std::min(speed[i],reachable);
 }
 result.count=count;
 for(std::size_t i=0;i<count;++i){
  auto&out=result.points[i];out.position=path[i].position;out.curvature_per_m=curv[i];out.speed_mps=speed[i];
  const auto wheels=wheel_speeds(speed[i],curv[i]);
  out.left_wheel_speed_mps=wheels.left_mps;out.right_wheel_speed_mps=wheels.right_mps;
  const double decel=std::max(0.05,constraints_.maximum_braking_deceleration_mps2+
                                  constraints_.gravity_mps2*std::sin(path[i].grade_rad));
  out.turn_anticipation_distance_m=turn_anticipation_distance(speed[i],decel);
  out.braking_distance_m=std::max(0.0,speed[i]*speed[i]-speed[count-1]*speed[count-1])/(2.0*decel);
  if(i>0){const double average=0.5*(speed[i]+speed[i-1]);if(average>kEpsilon)result.estimated_time_seconds+=ds[i]/average;}
 }
 result.valid=std::isfinite(result.estimated_time_seconds);
 return result;
}
} // namespace oblivion
