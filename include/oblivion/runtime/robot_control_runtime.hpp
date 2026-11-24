#pragma once
#include "oblivion/control/parallel_controller_bank.hpp"
#include "oblivion/core/types.hpp"
#include "oblivion/perception/reflectance_processor.hpp"
#include "oblivion/safety/motor_limiter.hpp"
namespace oblivion {
struct ControlRuntimeConfig {
 double cruise_command{0.25};
 double minimum_battery_voltage{5.5};
 double maximum_sensor_age_seconds{0.020};
 double maximum_line_confidence{0.15};
 double maximum_steering{0.8};
};
struct RuntimeDiagnostics {
 LineObservation line{};
 ControllerConsensus consensus{};
 MotorCommand requested{};
 MotorCommand applied{};
 double line_rate_mps{0.0};
 double dt_seconds{0.0};
 bool frame_valid{false};
 bool line_valid{false};
 bool battery_valid{false};
 bool safe{false};
};
class RobotControlRuntime {
public:
 RobotControlRuntime(ReflectanceProcessor perception,ParallelControllerBank& controllers,MotorLimiter limiter,ControlRuntimeConfig config={});
 MotorCommand process(const SensorFrame& frame,std::uint64_t now_us) noexcept;
 void emergency_stop() noexcept;
 [[nodiscard]] const RuntimeDiagnostics& diagnostics()const noexcept;
private:
 ReflectanceProcessor perception_;
 ParallelControllerBank& controllers_;
 MotorLimiter limiter_;
 ControlRuntimeConfig config_;
 RuntimeDiagnostics diagnostics_{};
 std::uint64_t previous_timestamp_us_{0};
 double previous_line_position_m_{0.0};
 bool initialized_{false};
 bool emergency_stopped_{false};
};
} // namespace oblivion
