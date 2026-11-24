#include "oblivion/control/parallel_controller_bank.hpp"
#include <cmath>
#include <stdexcept>
namespace oblivion_test {
class FixedController final:public oblivion::IController{
public: explicit FixedController(double value):value_(value){}
 oblivion::ControlOutput update(const oblivion::ControlInput&)noexcept override{return {value_,true};}
 void reset()noexcept override{}
private:double value_;
};
}
void test_parallel_controller_bank(){
 using namespace oblivion;
 oblivion_test::FixedController a(0.4),b(-0.2),c(0.1);
 std::vector<ControllerSlot> slots{{"positive",&a,1.0},{"inverted",&b,-1.0},{"third",&c,1.0}};
 ControllerBankConfig cfg{};cfg.deadline=std::chrono::milliseconds(100);cfg.minimum_valid_controllers=3;cfg.maximum_disagreement=0.5;
 ParallelControllerBank bank(std::move(slots),cfg);
 const auto result=bank.evaluate({0.1,0.0,0.001});
 if(!result.valid||result.valid_count!=3||result.total_count!=3)throw std::runtime_error("parallel controller consensus invalid");
 if(std::abs(result.command-0.2)>1e-9)throw std::runtime_error("controller polarity normalization or median incorrect");
 if(result.disagreement_exceeded||result.deadline_missed)throw std::runtime_error("unexpected controller bank fault");
 bank.reset();
 oblivion_test::FixedController far(0.9);
 ParallelControllerBank disagree({{"a",&a,1.0},{"b",&far,1.0}},ControllerBankConfig{std::chrono::milliseconds(100),2,0.2});
 const auto rejected=disagree.evaluate({0.1,0.0,0.001});
 if(rejected.valid||!rejected.disagreement_exceeded)throw std::runtime_error("controller disagreement was not rejected");
}
