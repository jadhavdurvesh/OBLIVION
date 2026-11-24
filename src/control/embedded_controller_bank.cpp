#include "oblivion/control/embedded_controller_bank.hpp"
#include <algorithm>
#include <cmath>
namespace oblivion {
bool EmbeddedControllerBank::configure(const EmbeddedControllerSlot* slots,std::size_t count,EmbeddedBankConfig config)noexcept{
 if(slots==nullptr||count==0||count>kMaxEmbeddedControllers||config.minimum_valid==0||config.minimum_valid>count||
 !std::isfinite(config.maximum_disagreement)||config.maximum_disagreement<0||config.maximum_execution_us==0)return false;
 for(std::size_t i=0;i<count;++i)if(slots[i].name==nullptr||slots[i].controller==nullptr||
 !std::isfinite(slots[i].polarity)||std::abs(slots[i].polarity)!=1.0)return false;
 count_=count;config_=config;for(std::size_t i=0;i<count;++i)slots_[i]=slots[i];reset();return true;
}
EmbeddedBankResult EmbeddedControllerBank::evaluate(const ControlInput& input,std::uint32_t elapsed)noexcept{
 EmbeddedBankResult result{};result.total_count=count_;result.execution_us=elapsed;
 if(count_==0||!std::isfinite(input.error)||!std::isfinite(input.measured_rate)||!std::isfinite(input.dt_seconds)||input.dt_seconds<=0)return result;
 if(elapsed>config_.maximum_execution_us){result.deadline_missed=true;return result;}
 for(std::size_t i=0;i<count_;++i){const auto out=slots_[i].controller->update(input);if(out.valid&&std::isfinite(out.command)){values_[result.valid_count++]=out.command*slots_[i].polarity;}}
 if(result.valid_count<config_.minimum_valid)return result;
 std::sort(values_.begin(),values_.begin()+static_cast<std::ptrdiff_t>(result.valid_count));
 result.disagreement=values_[result.valid_count-1]-values_[0];
 if(result.disagreement>config_.maximum_disagreement){result.disagreement_exceeded=true;return result;}
 const auto middle=result.valid_count/2;
 result.command=result.valid_count%2==0?0.5*(values_[middle-1]+values_[middle]):values_[middle];
 result.valid=std::isfinite(result.command);return result;
}
void EmbeddedControllerBank::reset()noexcept{for(std::size_t i=0;i<count_;++i)slots_[i].controller->reset();}
} // namespace oblivion
