#pragma once
#include "oblivion/core/types.hpp"
#include <array>
#include <cstddef>

namespace oblivion {
constexpr std::size_t kMaxTrajectoryPoints=512;

struct MotionConstraints {
 double maximum_speed_mps{1.5};
 double maximum_wheel_speed_mps{1.5};
 double maximum_lateral_acceleration_mps2{2.5};
 double maximum_longitudinal_acceleration_mps2{2.0};
 double maximum_braking_deceleration_mps2{3.0};
 double maximum_yaw_rate_radps{8.0};
 double wheel_track_m{0.12};
 double gravity_mps2{9.80665};
 double planning_latency_seconds{0.01};
 double safety_margin_m{0.01};
};
struct PathSample {
 MotionPoint2D position{};
 double grade_rad{0.0};
 double bank_rad{0.0};
};
struct PlannedMotionPoint {
 MotionPoint2D position{};
 double curvature_per_m{0.0};
 double speed_mps{0.0};
 double left_wheel_speed_mps{0.0};
 double right_wheel_speed_mps{0.0};
 double turn_anticipation_distance_m{0.0};
 double braking_distance_m{0.0};
};
struct MotionPlan {
 std::array<PlannedMotionPoint,kMaxTrajectoryPoints> points{};
 std::size_t count{0};
 double estimated_time_seconds{0.0};
 bool valid{false};
};
class MotionPlanner {
public:
 explicit MotionPlanner(MotionConstraints constraints) noexcept;
 [[nodiscard]] bool constraints_valid()const noexcept;
 [[nodiscard]] double curvature(MotionPoint2D a,MotionPoint2D b,MotionPoint2D c)const noexcept;
 [[nodiscard]] double maximum_corner_speed(double curvature_per_m,double grade_rad=0.0,double bank_rad=0.0)const noexcept;
 [[nodiscard]] double turn_anticipation_distance(double speed_mps,double deceleration_mps2)const noexcept;
 [[nodiscard]] WheelSpeeds wheel_speeds(double linear_speed_mps,double curvature_per_m)const noexcept;
 [[nodiscard]] MotionPlan plan(const PathSample* path,std::size_t count,double terminal_speed_mps=0.0)const noexcept;
private: MotionConstraints constraints_;
};
} // namespace oblivion
