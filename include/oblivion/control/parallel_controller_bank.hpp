#pragma once
#include "oblivion/control/controller.hpp"
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
namespace oblivion {
struct ControllerSlot { std::string name; IController* controller{nullptr}; double output_polarity{1.0}; };
struct ControllerBankConfig {
 std::chrono::microseconds deadline{std::chrono::microseconds(2000)};
 std::size_t minimum_valid_controllers{1};
 double maximum_disagreement{0.65};
};
struct ControllerCandidate { std::string name; ControlOutput output{}; };
struct ControllerConsensus {
 double command{0.0}; double disagreement{0.0};
 std::size_t valid_count{0}; std::size_t total_count{0};
 bool valid{false}; bool deadline_missed{false}; bool disagreement_exceeded{false};
 std::vector<ControllerCandidate> candidates;
};
class ParallelControllerBank {
public:
 explicit ParallelControllerBank(std::vector<ControllerSlot> slots, ControllerBankConfig config={});
 ~ParallelControllerBank();
 ParallelControllerBank(const ParallelControllerBank&)=delete;
 ParallelControllerBank& operator=(const ParallelControllerBank&)=delete;
 ControllerConsensus evaluate(const ControlInput& input);
 void reset();
 [[nodiscard]] std::size_t size() const noexcept;
private:
 struct WorkerState { ControllerSlot slot; ControlOutput output{}; std::thread thread; std::uint64_t observed_generation{0}; };
 void worker_loop(std::size_t index);
 std::vector<WorkerState> workers_;
 ControllerBankConfig config_;
 mutable std::mutex mutex_;
 std::condition_variable start_cv_, done_cv_;
 ControlInput input_{};
 std::uint64_t generation_{0};
 std::size_t remaining_{0};
 bool stopping_{false};
};
} // namespace oblivion
