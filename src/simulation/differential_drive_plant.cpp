#include "oblivion/simulation/differential_drive_plant.hpp"
#include "oblivion/kinematics/differential_drive.hpp"
#include <algorithm>
#include <cmath>

namespace oblivion {
DifferentialDrivePlant::DifferentialDrivePlant(PlantConfig c) noexcept:config_(c){}
bool DifferentialDrivePlant::configuration_valid()const noexcept{
 const auto& c=config_;
 return std::isfinite(c.wheel_radius_m)&&std::isfinite(c.track_width_m)&&
 std::isfinite(c.maximum_wheel_speed_mps)&&std::isfinite(c.maximum_wheel_acceleration_mps2)&&
 std::isfinite(c.motor_time_constant_seconds)&&std::isfinite(c.traction_factor)&&
 c.wheel_radius_m>0&&c.track_width_m>0&&c.maximum_wheel_speed_mps>0&&
 c.maximum_wheel_acceleration_mps2>0&&c.motor_time_constant_seconds>=0&&
 c.traction_factor>=0&&c.traction_factor<=1.0;
}
bool DifferentialDrivePlant::step(const MotorCommand& command,double dt)noexcept{
 if(!configuration_valid()||!std::isfinite(command.left)||!std::isfinite(command.right)||
    !std::isfinite(dt)||dt<=0||std::abs(command.left)>1.0||std::abs(command.right)>1.0)return false;
 const double target_left=command.left*config_.maximum_wheel_speed_mps;
 const double target_right=command.right*config_.maximum_wheel_speed_mps;
 const double alpha=config_.motor_time_constant_seconds==0.0?1.0:
     dt/(config_.motor_time_constant_seconds+dt);
 const auto update_wheel=[&](double current,double target)noexcept{
     const double desired_delta=(target-current)*alpha;
     const double maximum_delta=config_.maximum_wheel_acceleration_mps2*dt;
     return current+std::clamp(desired_delta,-maximum_delta,maximum_delta);
 };
 wheels_.left_mps=update_wheel(wheels_.left_mps,target_left)*config_.traction_factor;
 wheels_.right_mps=update_wheel(wheels_.right_mps,target_right)*config_.traction_factor;
 const DifferentialDriveModel model({config_.wheel_radius_m,config_.track_width_m});
 pose_=model.integrate(pose_,wheels_,dt);
 return std::isfinite(pose_.x_m)&&std::isfinite(pose_.y_m)&&std::isfinite(pose_.heading_rad);
}
const Pose2D& DifferentialDrivePlant::pose()const noexcept{return pose_;}
WheelSpeeds DifferentialDrivePlant::wheel_speeds()const noexcept{return wheels_;}
void DifferentialDrivePlant::reset(const Pose2D& p)noexcept{pose_=p;wheels_={};}
} // namespace oblivion
