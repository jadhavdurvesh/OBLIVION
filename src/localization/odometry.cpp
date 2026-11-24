#include "oblivion/localization/odometry.hpp"
#include <cmath>
namespace oblivion {
namespace {double wrap(double a)noexcept{constexpr double pi=3.14159265358979323846;while(a>pi)a-=2*pi;while(a< -pi)a+=2*pi;return a;}}
DifferentialDriveOdometry::DifferentialDriveOdometry(OdometryConfig c)noexcept:config_(c){}
bool DifferentialDriveOdometry::configuration_valid()const noexcept{return std::isfinite(config_.wheel_radius_m)&&config_.wheel_radius_m>0&&std::isfinite(config_.track_width_m)&&config_.track_width_m>0&&std::isfinite(config_.gyro_weight)&&config_.gyro_weight>=0&&config_.gyro_weight<=1;}
void DifferentialDriveOdometry::reset(Pose2D p)noexcept{state_={};state_.pose=p;state_.valid=configuration_valid()&&std::isfinite(p.x_m)&&std::isfinite(p.y_m)&&std::isfinite(p.heading_rad);}
OdometryState DifferentialDriveOdometry::update(double dl,double dr,double gyro,double dt,bool gyro_valid)noexcept{
 if(!configuration_valid()||!std::isfinite(dl)||!std::isfinite(dr)||!std::isfinite(dt)||dt<=0||dt>0.1||(gyro_valid&&!std::isfinite(gyro))){state_.valid=false;return state_;}
 const double distance=0.5*(dl+dr);const double encoder_yaw=(dr-dl)/config_.track_width_m;
 const double gyro_yaw=gyro_valid?gyro*dt:encoder_yaw;
 const double yaw=gyro_valid?config_.gyro_weight*gyro_yaw+(1-config_.gyro_weight)*encoder_yaw:encoder_yaw;
 const double mid=state_.pose.heading_rad+0.5*yaw;
 state_.pose.x_m+=distance*std::cos(mid);state_.pose.y_m+=distance*std::sin(mid);
 state_.pose.heading_rad=wrap(state_.pose.heading_rad+yaw);
 state_.linear_speed_mps=distance/dt;state_.angular_speed_radps=yaw/dt;
 state_.heading_uncertainty_rad=gyro_valid?state_.heading_uncertainty_rad*0.98+0.0005:state_.heading_uncertainty_rad+std::abs(yaw)*0.01;
 state_.valid=std::isfinite(state_.pose.x_m)&&std::isfinite(state_.pose.y_m)&&std::isfinite(state_.pose.heading_rad);
 return state_;
}
} // namespace oblivion
