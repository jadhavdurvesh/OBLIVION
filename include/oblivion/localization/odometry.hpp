#pragma once
#include "oblivion/core/types.hpp"
namespace oblivion {
struct OdometryConfig{double wheel_radius_m{0.02};double track_width_m{0.12};double gyro_weight{0.96};};
struct OdometryState{Pose2D pose{};double linear_speed_mps{0};double angular_speed_radps{0};double heading_uncertainty_rad{0};bool valid{false};};
class DifferentialDriveOdometry{
public:explicit DifferentialDriveOdometry(OdometryConfig config)noexcept;
 [[nodiscard]] bool configuration_valid()const noexcept;
 void reset(Pose2D pose={})noexcept;
 OdometryState update(double left_wheel_distance_m,double right_wheel_distance_m,double gyro_z_radps,double dt_seconds,bool gyro_valid)noexcept;
 [[nodiscard]] const OdometryState& state()const noexcept{return state_;}
private:OdometryConfig config_;OdometryState state_{};
};
} // namespace oblivion
