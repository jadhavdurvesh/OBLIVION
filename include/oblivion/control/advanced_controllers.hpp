#pragma once

#include "oblivion/control/controller.hpp"
#include <array>
#include <cstddef>

namespace oblivion {

struct OutputLimits { double minimum{-1.0}; double maximum{1.0}; };
struct LqrGains { double position{1.0}; double rate{0.2}; };
class LqrController final : public IController {
public:
    LqrController(LqrGains gains, OutputLimits limits) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: LqrGains gains_; OutputLimits limits_;
};

struct MpcConfig {
    double position_weight{8.0};
    double rate_weight{1.0};
    double effort_weight{0.2};
    double horizon_seconds{0.25};
    double maximum_acceleration{4.0};
    unsigned candidate_count{ 9 };
    OutputLimits limits{};
};
class MpcController final : public IController {
public:
    explicit MpcController(MpcConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: MpcConfig config_;
};

struct SlidingModeConfig { double lambda{3.0}; double switching_gain{0.8}; double boundary_layer{0.05}; double rate_gain{0.2}; OutputLimits limits{}; };
class SlidingModeController final : public IController {
public:
    explicit SlidingModeController(SlidingModeConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: SlidingModeConfig config_;
};

struct AdrcConfig { double observer_bandwidth{18.0}; double control_gain{5.0}; double disturbance_gain{1.0}; OutputLimits limits{}; };
class AdrcController final : public IController {
public:
    explicit AdrcController(AdrcConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private:
    AdrcConfig config_;
    double estimated_error_{0.0};
    double estimated_disturbance_{0.0};
    bool initialized_{false};
};

struct BacksteppingConfig { double position_gain{2.0}; double rate_gain{1.0}; OutputLimits limits{}; };
class BacksteppingController final : public IController {
public:
    explicit BacksteppingController(BacksteppingConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: BacksteppingConfig config_;
};

struct AdaptivePdConfig {
    double initial_position_gain{1.0};
    double initial_rate_gain{0.2};
    double adaptation_rate{0.05};
    double minimum_position_gain{0.1};
    double maximum_position_gain{8.0};
    double minimum_rate_gain{0.01};
    double maximum_rate_gain{3.0};
    OutputLimits limits{};
};
class AdaptivePdController final : public IController {
public:
    explicit AdaptivePdController(AdaptivePdConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
    [[nodiscard]] double position_gain() const noexcept;
    [[nodiscard]] double rate_gain() const noexcept;
private:
    AdaptivePdConfig config_;
    double kp_;
    double kd_;
};

struct GainSchedulePoint { double absolute_error{0.0}; double position_gain{1.0}; double rate_gain{0.2}; };
template <std::size_t N>
struct GainSchedule { std::array<GainSchedulePoint, N> points{}; OutputLimits limits{}; };


struct GainScheduleConfig {
    std::array<GainSchedulePoint, 16> points{};
    std::size_t point_count{0};
    OutputLimits limits{};
};
class GainScheduledController final : public IController {
public:
    explicit GainScheduledController(GainScheduleConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: GainScheduleConfig config_;
};

class FuzzyController final : public IController {
public:
    FuzzyController(double error_scale, double rate_scale, OutputLimits limits) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: double error_scale_; double rate_scale_; OutputLimits limits_;
};

// Runtime robust feedback using pre-computed state-feedback gains. This class
// does NOT synthesize an H-infinity controller; synthesis is an offline design task.
struct RobustFeedbackConfig { double position_gain{2.0}; double rate_gain{0.5}; double uncertainty_bound{0.2}; OutputLimits limits{}; };
class RobustFeedbackController final : public IController {
public:
    explicit RobustFeedbackController(RobustFeedbackConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: RobustFeedbackConfig config_;
};

class ControllerSupervisor {
public:
    explicit ControllerSupervisor(OutputLimits limits) noexcept;
    void select(IController* controller) noexcept;
    ControlOutput update(const ControlInput&) noexcept;
    void reset() noexcept;
    [[nodiscard]] bool has_controller() const noexcept;
private: IController* active_{nullptr}; OutputLimits limits_;
};

} // namespace oblivion
