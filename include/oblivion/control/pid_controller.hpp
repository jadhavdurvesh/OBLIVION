#pragma once
#include "oblivion/control/controller.hpp"
namespace oblivion {
struct PidGains { double kp{0.0}; double ki{0.0}; double kd{0.0}; };
struct PidLimits { double output_min{-1.0}; double output_max{1.0}; double integral_min{-1.0}; double integral_max{1.0}; };
class PidController final : public IController {
public:
    PidController(PidGains gains, PidLimits limits) noexcept;
    ControlOutput update(const ControlInput& input) noexcept override;
    void reset() noexcept override;
private:
    PidGains gains_;
    PidLimits limits_;
    double integral_{0.0};
    double previous_error_{0.0};
    bool has_previous_{false};
};
} // namespace oblivion
