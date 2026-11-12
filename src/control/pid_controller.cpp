#include "oblivion/control/pid_controller.hpp"
#include <algorithm>
#include <cmath>
namespace oblivion {
namespace { double clamp_value(double v, double lo, double hi) noexcept { return std::max(lo, std::min(v, hi)); } }
PidController::PidController(PidGains gains, PidLimits limits) noexcept : gains_(gains), limits_(limits) {}
ControlOutput PidController::update(const ControlInput& input) noexcept {
    if (!std::isfinite(input.error) || !std::isfinite(input.measured_rate) ||
        !std::isfinite(input.dt_seconds) || input.dt_seconds <= 0.0 ||
        limits_.output_min > limits_.output_max || limits_.integral_min > limits_.integral_max) return {0.0, false};
    const double candidate_integral = clamp_value(integral_ + input.error * input.dt_seconds,
                                                   limits_.integral_min, limits_.integral_max);
    const double derivative = has_previous_ ? (input.error - previous_error_) / input.dt_seconds : 0.0;
    const double unconstrained = gains_.kp * input.error + gains_.ki * candidate_integral + gains_.kd * derivative;
    const double constrained = clamp_value(unconstrained, limits_.output_min, limits_.output_max);
    const bool drives_high = unconstrained > limits_.output_max && input.error > 0.0;
    const bool drives_low = unconstrained < limits_.output_min && input.error < 0.0;
    if (!drives_high && !drives_low) integral_ = candidate_integral;
    previous_error_ = input.error;
    has_previous_ = true;
    if (!std::isfinite(constrained)) { reset(); return {0.0, false}; }
    return {constrained, true};
}
void PidController::reset() noexcept { integral_ = 0.0; previous_error_ = 0.0; has_previous_ = false; }
} // namespace oblivion
