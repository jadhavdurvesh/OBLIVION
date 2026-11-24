#include "oblivion/control/parallel_controller_bank.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace oblivion {
ParallelControllerBank::ParallelControllerBank(std::vector<ControllerSlot> slots,ControllerBankConfig config):config_(config){
 if(slots.empty()||config_.deadline.count()<=0||config_.minimum_valid_controllers==0||config_.minimum_valid_controllers>slots.size()||!std::isfinite(config_.maximum_disagreement)||config_.maximum_disagreement<0.0)throw std::invalid_argument("invalid parallel controller bank configuration");
 workers_.reserve(slots.size());
 for(auto& slot:slots){if(slot.name.empty()||slot.controller==nullptr)throw std::invalid_argument("controller slot requires a name and controller");WorkerState state{};state.slot=std::move(slot);workers_.push_back(std::move(state));}
 for(std::size_t i=0;i<workers_.size();++i)workers_[i].thread=std::thread(&ParallelControllerBank::worker_loop,this,i);
}
ParallelControllerBank::~ParallelControllerBank(){
 {std::lock_guard<std::mutex> lock(mutex_);stopping_=true;}
 start_cv_.notify_all();
 for(auto& worker:workers_)if(worker.thread.joinable())worker.thread.join();
}
void ParallelControllerBank::worker_loop(std::size_t index){
 auto& worker=workers_[index];
 for(;;){
  ControlInput input{};
  {std::unique_lock<std::mutex> lock(mutex_);start_cv_.wait(lock,[&]{return stopping_||generation_!=worker.observed_generation;});if(stopping_)return;worker.observed_generation=generation_;input=input_;}
  const auto output=worker.slot.controller->update(input);
  {std::lock_guard<std::mutex> lock(mutex_);worker.output=output;if(remaining_>0)--remaining_;if(remaining_==0)done_cv_.notify_one();}
 }
}
ControllerConsensus ParallelControllerBank::evaluate(const ControlInput& input){
 ControllerConsensus result{};result.total_count=workers_.size();result.candidates.reserve(workers_.size());
 if(!std::isfinite(input.error)||!std::isfinite(input.measured_rate)||!std::isfinite(input.dt_seconds)||input.dt_seconds<=0.0)return result;
 std::unique_lock<std::mutex> lock(mutex_);
 if(stopping_||remaining_!=0){result.deadline_missed=true;return result;}
 input_=input;remaining_=workers_.size();++generation_;start_cv_.notify_all();
 const bool completed=done_cv_.wait_for(lock,config_.deadline,[&]{return remaining_==0;});
 result.deadline_missed=!completed;
 std::vector<double> values;values.reserve(workers_.size());
 for(const auto& worker:workers_){result.candidates.push_back({worker.slot.name,worker.output});if(worker.output.valid&&std::isfinite(worker.output.command)){++result.valid_count;values.push_back(worker.output.command);}}
 if(!completed||values.size()<config_.minimum_valid_controllers)return result;
 std::sort(values.begin(),values.end());
 const double low=values.front(),high=values.back();result.disagreement=high-low;
 if(result.disagreement>config_.maximum_disagreement){result.disagreement_exceeded=true;return result;}
 const auto middle=values.size()/2;
 result.command=(values.size()%2==0)?0.5*(values[middle-1]+values[middle]):values[middle];
 result.valid=std::isfinite(result.command);
 return result;
}
void ParallelControllerBank::reset(){
 std::unique_lock<std::mutex> lock(mutex_);
 if(remaining_!=0)return;
 for(auto& worker:workers_)worker.slot.controller->reset();
}
std::size_t ParallelControllerBank::size()const noexcept{return workers_.size();}
} // namespace oblivion
