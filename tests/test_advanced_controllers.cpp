#include "oblivion/control/advanced_controllers.hpp"
#include <cmath>
#include <stdexcept>
namespace { void require(bool c,const char*m){if(!c)throw std::runtime_error(m);} }
void test_advanced_controllers(){
 using namespace oblivion; const OutputLimits lim{-1.0,1.0};
 const ControlInput in{0.2,0.1,0.01};
 LqrController lqr({2.0,0.5},lim); require(lqr.update(in).valid,"LQR invalid");
 MpcConfig mc{}; mc.limits=lim; MpcController mpc(mc); require(mpc.update(in).valid,"MPC invalid");
 SlidingModeController smc({3.0,0.8,0.05,0.2,lim}); require(smc.update(in).valid,"SMC invalid");
 AdrcController adrc({18.0,5.0,1.0,lim}); require(adrc.update(in).valid,"ADRC invalid");
 BacksteppingController back({2.0,1.0,lim}); require(back.update(in).valid,"backstepping invalid");
 AdaptivePdController adaptive({1.0,0.2,0.05,0.1,8.0,0.01,3.0,lim});
 require(adaptive.update(in).valid,"adaptive PD invalid");
 const double old_gain=adaptive.position_gain(); adaptive.update({0.5,0.2,0.1});
 require(adaptive.position_gain()>=old_gain,"adaptive gain did not adapt");
 GainScheduleConfig gs{}; gs.point_count=2; gs.limits=lim;
 gs.points[0]={0.0,1.0,0.1}; gs.points[1]={1.0,3.0,0.5};
 GainScheduledController scheduled(gs); require(scheduled.update(in).valid,"gain schedule invalid");
 FuzzyController fuzzy(0.5,1.0,lim); require(fuzzy.update(in).valid,"fuzzy invalid");
 RobustFeedbackController robust({2.0,0.5,0.2,lim}); require(robust.update(in).valid,"robust feedback invalid");
 ControllerSupervisor supervisor(lim); require(!supervisor.has_controller(),"supervisor unexpectedly active");
 supervisor.select(&lqr); require(supervisor.has_controller(),"supervisor failed to select controller");
 require(supervisor.update(in).valid,"supervisor rejected valid output");
 supervisor.select(nullptr); require(!supervisor.update(in).valid,"supervisor must fail closed without controller");
 const ControlInput bad{NAN,0.0,0.01}; require(!lqr.update(bad).valid,"LQR accepted NaN");
}
