#pragma once
#include "oblivion/control/controller.hpp"
#include <array>
#include <cstdint>
#include <cstddef>
namespace oblivion {
constexpr std::size_t kMaxEmbeddedControllers=12;
struct EmbeddedControllerSlot{const char* name{nullptr};IController* controller{nullptr};double polarity{1.0};};
struct EmbeddedBankConfig{std::size_t minimum_valid{1};double maximum_disagreement{0.5};std::uint32_t maximum_execution_us{1000};};
struct EmbeddedBankResult{double command{0};double disagreement{0};std::size_t valid_count{0};std::size_t total_count{0};std::uint32_t execution_us{0};bool valid{false};bool deadline_missed{false};bool disagreement_exceeded{false};};
class EmbeddedControllerBank{
public:
 bool configure(const EmbeddedControllerSlot* slots,std::size_t count,EmbeddedBankConfig config)noexcept;
 EmbeddedBankResult evaluate(const ControlInput& input,std::uint32_t measured_execution_us)noexcept;
 void reset()noexcept;
 [[nodiscard]] std::size_t size()const noexcept{return count_;}
private:
 std::array<EmbeddedControllerSlot,kMaxEmbeddedControllers> slots_{};
 std::array<double,kMaxEmbeddedControllers> values_{};
 std::size_t count_{0};EmbeddedBankConfig config_{};
};
} // namespace oblivion
