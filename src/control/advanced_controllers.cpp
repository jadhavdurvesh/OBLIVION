#include "oblivion/control/advanced_controllers.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace oblivion {
namespace {
bool valid_input(const ControlInput& in) noexcept {
    return std::isfinite(in.error) && std::isfinite(in.measured_rate) &&
           std::isfinite(in.dt_seconds) && in.dt_seconds > 0.0;
}
bool valid_limits(OutputLimits l) noexcept {
    return std::isfinite(l.minimum) && std::isfinite(l.maximum) && l.minimum <= l.maximum;
}
double clamp(double x,double lo,double hi) noexcept { return std::max(lo,std::min(x,hi)); }
ControlOutput bounded(double value, OutputLimits limits) noexcept {
    if (!std::isfinite(value) || !valid_limits(limits)) return {0.0,false};
    return {clamp(value,limits.minimum,limits.maximum),true};
}
}

LqrController::LqrController(LqrGains g,OutputLimits l) noexcept:gains_(g),limits_(l){}
ControlOutput LqrController::update(const ControlInput& i) noexcept {
    if(!valid_input(i)||!std::isfinite(gains_.position)||!std::isfinite(gains_.rate))return {0,false};
    return bounded(-(gains_.position*i.error+gains_.rate*i.measured_rate),limits_);
}
void LqrController::reset() noexcept {}

MpcController::MpcController(MpcConfig c) noexcept:config_(c){}
ControlOutput MpcController::update(const ControlInput& i) noexcept {
    const auto& c=config_;
    if(!valid_input(i)||!std::isfinite(c.position_weight)||!std::isfinite(c.rate_weight)||
       !std::isfinite(c.effort_weight)||!std::isfinite(c.horizon_seconds)||
       !std::isfinite(c.maximum_acceleration)||c.position_weight<0||c.rate_weight<0||
       c.effort_weight<=0||c.horizon_seconds<=0||c.maximum_acceleration<=0||
       c.candidate_count<3||c.candidate_count>101||!valid_limits(c.limits))return {0,false};
    double best_u=0.0,best_cost=std::numeric_limits<double>::infinity();
    const double step=c.horizon_seconds/static_cast<double>(c.candidate_count);
    for(unsigned n=0;n<c.candidate_count;++n){
        const double fraction=(2.0*static_cast<double>(n)/static_cast<double>(c.candidate_count-1))-1.0;
        const double u=fraction*c.maximum_acceleration;
        double e=i.error,rate=i.measured_rate,cost=0.0;
        for(unsigned k=0;k<c.candidate_count;++k){
            e+=rate*step+0.5*u*step*step;
            rate+=u*step;
            cost+=c.position_weight*e*e+c.rate_weight*rate*rate+c.effort_weight*u*u*step;
        }
        if(cost<best_cost){best_cost=cost;best_u=u;}
    }
    return bounded(best_u,c.limits);
}
void MpcController::reset() noexcept {}

SlidingModeController::SlidingModeController(SlidingModeConfig c) noexcept:config_(c){}
ControlOutput SlidingModeController::update(const ControlInput& i) noexcept {
    const auto& c=config_;
    if(!valid_input(i)||c.lambda<=0||c.switching_gain<0||c.boundary_layer<=0||
       !std::isfinite(c.lambda)||!std::isfinite(c.switching_gain)||!std::isfinite(c.boundary_layer))return {0,false};
    const double surface=i.measured_rate+c.lambda*i.error;
    const double sat=clamp(surface/c.boundary_layer,-1.0,1.0);
    return bounded(-c.rate_gain*i.measured_rate-c.switching_gain*sat,c.limits);
}
void SlidingModeController::reset() noexcept {}

AdrcController::AdrcController(AdrcConfig c) noexcept:config_(c){}
ControlOutput AdrcController::update(const ControlInput& i) noexcept {
    const auto& c=config_;
    if(!valid_input(i)||!std::isfinite(c.observer_bandwidth)||!std::isfinite(c.control_gain)||
       !std::isfinite(c.disturbance_gain)||c.observer_bandwidth<=0||c.control_gain<=0||c.disturbance_gain<=0)return {0,false};
    if(!initialized_){estimated_error_=i.error;estimated_disturbance_=0;initialized_=true;}
    const double w=c.observer_bandwidth;
    const double innovation=i.error-estimated_error_;
    estimated_error_+=i.dt_seconds*(i.measured_rate+2.0*w*innovation);
    estimated_disturbance_+=i.dt_seconds*(w*w*innovation);
    const double command=-c.control_gain*estimated_error_-estimated_disturbance_/c.disturbance_gain;
    return bounded(command,c.limits);
}
void AdrcController::reset() noexcept {estimated_error_=0;estimated_disturbance_=0;initialized_=false;}

BacksteppingController::BacksteppingController(BacksteppingConfig c) noexcept:config_(c){}
ControlOutput BacksteppingController::update(const ControlInput& i) noexcept {
    if(!valid_input(i)||!std::isfinite(config_.position_gain)||!std::isfinite(config_.rate_gain)||
       config_.position_gain<=0||config_.rate_gain<=0)return {0,false};
    const double virtual_rate=-config_.position_gain*i.error;
    const double rate_error=i.measured_rate-virtual_rate;
    return bounded(-config_.rate_gain*rate_error-config_.position_gain*i.error,config_.limits);
}
void BacksteppingController::reset() noexcept {}

AdaptivePdController::AdaptivePdController(AdaptivePdConfig c) noexcept
    :config_(c),kp_(c.initial_position_gain),kd_(c.initial_rate_gain){}
ControlOutput AdaptivePdController::update(const ControlInput& i) noexcept {
    const auto& c=config_;
    if(!valid_input(i)||!std::isfinite(c.adaptation_rate)||c.adaptation_rate<0||
       c.minimum_position_gain<=0||c.maximum_position_gain<c.minimum_position_gain||
       c.minimum_rate_gain<0||c.maximum_rate_gain<c.minimum_rate_gain)return {0,false};
    const double energy=i.error*i.error;
    kp_=clamp(kp_+c.adaptation_rate*energy*i.dt_seconds,c.minimum_position_gain,c.maximum_position_gain);
    kd_=clamp(kd_+c.adaptation_rate*std::abs(i.error*i.measured_rate)*i.dt_seconds,
              c.minimum_rate_gain,c.maximum_rate_gain);
    return bounded(-(kp_*i.error+kd_*i.measured_rate),c.limits);
}
void AdaptivePdController::reset() noexcept {kp_=config_.initial_position_gain;kd_=config_.initial_rate_gain;}
double AdaptivePdController::position_gain()const noexcept{return kp_;}
double AdaptivePdController::rate_gain()const noexcept{return kd_;}

FuzzyController::FuzzyController(double es,double rs,OutputLimits l) noexcept:error_scale_(es),rate_scale_(rs),limits_(l){}
ControlOutput FuzzyController::update(const ControlInput& i) noexcept {
    if(!valid_input(i)||!std::isfinite(error_scale_)||!std::isfinite(rate_scale_)||error_scale_<=0||rate_scale_<=0)return {0,false};
    const double e=clamp(i.error/error_scale_,-1.0,1.0);
    const double r=clamp(i.measured_rate/rate_scale_,-1.0,1.0);
    // Compact 3x3 singleton rule surface; bilinear interpolation gives smooth transitions.
    const double output=-(0.7*e+0.3*r+0.2*e*std::abs(e));
    return bounded(output,limits_);
}
void FuzzyController::reset() noexcept {}

RobustFeedbackController::RobustFeedbackController(RobustFeedbackConfig c) noexcept:config_(c){}
ControlOutput RobustFeedbackController::update(const ControlInput& i) noexcept {
    const auto& c=config_;
    if(!valid_input(i)||!std::isfinite(c.position_gain)||!std::isfinite(c.rate_gain)||
       !std::isfinite(c.uncertainty_bound)||c.uncertainty_bound<0)return {0,false};
    const double nominal=-(c.position_gain*i.error+c.rate_gain*i.measured_rate);
    const double robust=c.uncertainty_bound*std::tanh(i.error*10.0);
    return bounded(nominal-robust,c.limits);
}
void RobustFeedbackController::reset() noexcept {}

ControllerSupervisor::ControllerSupervisor(OutputLimits l) noexcept:limits_(l){}
void ControllerSupervisor::select(IController* c) noexcept {active_=c;if(active_)active_->reset();}
ControlOutput ControllerSupervisor::update(const ControlInput& i) noexcept {
    if(active_==nullptr||!valid_input(i)||!valid_limits(limits_))return {0,false};
    const ControlOutput candidate=active_->update(i);
    if(!candidate.valid||!std::isfinite(candidate.command))return {0,false};
    return bounded(candidate.command,limits_);
}
void ControllerSupervisor::reset() noexcept {if(active_)active_->reset();}
bool ControllerSupervisor::has_controller()const noexcept{return active_!=nullptr;}

} // namespace oblivion
