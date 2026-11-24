#include "oblivion/safety/motor_slew_limiter.hpp"
#include <algorithm>
#include <cmath>
namespace oblivion {
MotorSlewLimiter::MotorSlewLimiter(double rate)noexcept:maximum_delta_per_second_(rate){}
bool MotorSlewLimiter::configuration_valid()const noexcept{return std::isfinite(maximum_delta_per_second_)&&maximum_delta_per_second_>0;}
MotorCommand MotorSlewLimiter::apply(MotorCommand requested,double dt)noexcept{
 if(!configuration_valid()||!std::isfinite(dt)||dt<=0||!std::isfinite(requested.left)||!std::isfinite(requested.right)){reset();return {};}
 const double delta=maximum_delta_per_second_*dt;
 previous_.left+=std::clamp(requested.left-previous_.left,-delta,delta);
 previous_.right+=std::clamp(requested.right-previous_.right,-delta,delta);
 return previous_;
}
void MotorSlewLimiter::reset()noexcept{previous_={};}
} // namespace oblivion
