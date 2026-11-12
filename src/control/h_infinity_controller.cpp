#include "oblivion/control/h_infinity_controller.hpp"
#include <cmath>
#include <algorithm>
namespace oblivion {
HInfinityController::HInfinityController(HInfinityConfig c) noexcept:config_(c){}
ControlOutput HInfinityController::update(const ControlInput& i) noexcept {
    const auto& c=config_;
    if(!std::isfinite(i.error)||!std::isfinite(i.measured_rate)||!std::isfinite(i.dt_seconds)||
       i.dt_seconds<=0.0||!std::isfinite(c.gain_error)||!std::isfinite(c.gain_rate)||
       !std::isfinite(c.maximum_state_magnitude)||c.maximum_state_magnitude<=0.0||
       !std::isfinite(c.limits.minimum)||!std::isfinite(c.limits.maximum)||
       c.limits.minimum>c.limits.maximum||
       std::abs(i.error)>c.maximum_state_magnitude||
       std::abs(i.measured_rate)>c.maximum_state_magnitude)return {0.0,false};
    const double u=-(c.gain_error*i.error+c.gain_rate*i.measured_rate);
    if(!std::isfinite(u))return {0.0,false};
    return {std::max(c.limits.minimum,std::min(u,c.limits.maximum)),true};
}
void HInfinityController::reset() noexcept {}
} // namespace oblivion
