#pragma once
#include "oblivion/control/controller.hpp"
#include "oblivion/control/advanced_controllers.hpp"
#include <array>

namespace oblivion {

// Runtime realization of an offline-synthesized continuous-time H-infinity
// state-feedback gain. State convention: [line_error, line_error_rate].
struct HInfinityConfig {
    double gain_error{0.0};
    double gain_rate{0.0};
    OutputLimits limits{};
    double maximum_state_magnitude{100.0};
};
class HInfinityController final : public IController {
public:
    explicit HInfinityController(HInfinityConfig config) noexcept;
    ControlOutput update(const ControlInput&) noexcept override;
    void reset() noexcept override;
private: HInfinityConfig config_;
};

} // namespace oblivion
