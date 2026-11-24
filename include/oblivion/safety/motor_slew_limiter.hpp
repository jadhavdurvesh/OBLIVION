#pragma once
#include "oblivion/core/types.hpp"
namespace oblivion {
class MotorSlewLimiter{
public:
 explicit MotorSlewLimiter(double maximum_delta_per_second)noexcept;
 [[nodiscard]] bool configuration_valid()const noexcept;
 MotorCommand apply(MotorCommand requested,double dt_seconds)noexcept;
 void reset()noexcept;
private:double maximum_delta_per_second_;MotorCommand previous_{};
};
} // namespace oblivion
