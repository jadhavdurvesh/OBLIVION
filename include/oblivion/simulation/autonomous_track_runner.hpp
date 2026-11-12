#pragma once
#include "oblivion/control/pid_controller.hpp"
#include "oblivion/motion/planner.hpp"
#include "oblivion/perception/reflectance_processor.hpp"
#include "oblivion/safety/motor_limiter.hpp"
#include "oblivion/simulation/differential_drive_plant.hpp"
#include "oblivion/simulation/track_simulator.hpp"
#include <cstddef>
#include <cstdint>

namespace oblivion {
struct TrackRunnerConfig {
 PlantConfig plant{};
 MotionConstraints motion{};
 SimulatedReflectanceConfig simulated_sensors{};
 ReflectanceConfig perception{};
 PidGains steering_gains{8.0,0.0,0.12};
 PidLimits steering_limits{-0.65,0.65,-0.2,0.2};
 double motor_command_limit{1.0};
 double sensor_forward_offset_m{0.035};
 double minimum_tracking_confidence{0.2};
 double completion_radius_m{0.04};
 double maximum_run_time_seconds{30.0};
};
struct TrackRunMetrics {
 Pose2D final_pose{};
 double maximum_cross_track_error_m{0.0};
 double rms_cross_track_error_m{0.0};
 double mean_cross_track_error_m{0.0};
 double estimated_time_seconds{0.0};
 double simulated_time_seconds{0.0};
 std::size_t steps{0};
 std::size_t lost_line_steps{0};
 std::size_t sensor_fault_steps{0};
 std::size_t low_confidence_steps{0};
 bool completed{false};
 bool timed_out{false};
 bool faulted{false};
 bool valid{false};
 MotionPlan planned_motion{};
};
class AutonomousTrackRunner {
public:
 explicit AutonomousTrackRunner(TrackRunnerConfig config) noexcept;
 [[nodiscard]] bool configuration_valid()const noexcept;
 [[nodiscard]] TrackRunMetrics run(const Track& track,const Pose2D& initial_pose,
                                    double dt_seconds,std::size_t maximum_steps) noexcept;
private:
 TrackRunnerConfig config_;
 DifferentialDrivePlant plant_;
 ReflectanceArraySimulator sensor_simulator_;
 ReflectanceProcessor perception_;
 MotionPlanner planner_;
 PidController steering_;
 MotorLimiter motor_limiter_;
};
} // namespace oblivion
