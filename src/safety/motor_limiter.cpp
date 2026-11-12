#include "oblivion/safety/motor_limiter.hpp"
#include <algorithm>
#include <cmath>
namespace oblivion {
MotorLimiter::MotorLimiter(double absolute_limit) noexcept : absolute_limit_(absolute_limit) {}
MotorCommand MotorLimiter::apply(const MotorCommand& requested) const noexcept {
    if (!configuration_valid() || !std::isfinite(requested.left) || !std::isfinite(requested.right)) return {0.0, 0.0};
    return {std::clamp(requested.left, -absolute_limit_, absolute_limit_),
            std::clamp(requested.right, -absolute_limit_, absolute_limit_)};
}
bool MotorLimiter::configuration_valid() const noexcept { return std::isfinite(absolute_limit_) && absolute_limit_ >= 0.0; }
} // namespace oblivion
