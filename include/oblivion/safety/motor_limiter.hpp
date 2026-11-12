#pragma once
#include "oblivion/core/types.hpp"
namespace oblivion {
class MotorLimiter {
public:
    explicit MotorLimiter(double absolute_limit) noexcept;
    [[nodiscard]] MotorCommand apply(const MotorCommand& requested) const noexcept;
    [[nodiscard]] bool configuration_valid() const noexcept;
private:
    double absolute_limit_;
};
} // namespace oblivion
