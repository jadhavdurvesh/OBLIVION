#pragma once
#include "oblivion/core/types.hpp"
#include <cstddef>
namespace oblivion {
struct Vector3 { double x{0.0},y{0.0},z{0.0}; };
struct Quaternion { double w{1.0},x{0.0},y{0.0},z{0.0}; };
struct Pose3D { Vector3 position_m{}; Quaternion orientation{}; };
struct TerrainPlane3D { Vector3 point_m{}; Vector3 normal{0.0,0.0,1.0}; };
struct RigidBody3DConfig {
 double mass_kg{0.45};
 Vector3 inertia_kg_m2{0.002,0.004,0.005};
 Vector3 chassis_size_m{0.18,0.14,0.045};
 double wheel_radius_m{0.02};
 double wheel_track_m{0.12};
 double wheelbase_m{0.10};
 double wheel_inertia_kg_m2{0.00008};
 double maximum_motor_torque_nm{0.12};
 double motor_time_constant_s{0.025};
 double tire_longitudinal_stiffness_n_per_mps{18.0};
 double tire_lateral_stiffness_n_per_mps{10.0};
 double friction_coefficient{0.75};
 double suspension_stiffness_n_per_m{1800.0};
 double suspension_damping_n_s_per_m{22.0};
 double rolling_resistance{0.015};
 double aerodynamic_drag_n_per_mps2{0.02};
 double gravity_mps2{9.80665};
};
struct RigidBody3DState {
 Pose3D pose{};
 Vector3 linear_velocity_world_mps{};
 Vector3 angular_velocity_body_radps{};
 double left_wheel_radps{0.0};
 double right_wheel_radps{0.0};
 double left_motor_torque_nm{0.0};
 double right_motor_torque_nm{0.0};
 double time_s{0.0};
};
struct RigidBody3DInput {
 MotorCommand motors{};
 TerrainPlane3D terrain{};
 double battery_voltage_ratio{1.0};
};
struct RigidBody3DOutput {
 RigidBody3DState state{};
 Vector3 total_force_world_n{};
 Vector3 total_torque_body_nm{};
 double left_normal_load_n{0.0};
 double right_normal_load_n{0.0};
 double left_tire_force_n{0.0};
 double right_tire_force_n{0.0};
 double traction_utilization{0.0};
 std::size_t contact_points{0};
 bool airborne{false};
 bool valid{false};
};
class RigidBody3DSimulator {
public:
 explicit RigidBody3DSimulator(RigidBody3DConfig config={}) noexcept;
 [[nodiscard]] bool configuration_valid()const noexcept;
 [[nodiscard]] RigidBody3DOutput step(const RigidBody3DInput&,double dt_seconds) noexcept;
 [[nodiscard]] const RigidBody3DState& state()const noexcept;
 void reset(const RigidBody3DState& state={}) noexcept;
private:
 RigidBody3DConfig config_;
 RigidBody3DState state_{};
};
} // namespace oblivion
