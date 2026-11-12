#pragma once
namespace oblivion {
// measured_rate is the measured process-variable derivative; derivative-on-measurement
// avoids a derivative kick when the setpoint changes.
struct ControlInput { double error{0.0}; double measured_rate{0.0}; double dt_seconds{0.0}; };
struct ControlOutput { double command{0.0}; bool valid{false}; };
class IController {
public:
    virtual ~IController() = default;
    virtual ControlOutput update(const ControlInput& input) noexcept = 0;
    virtual void reset() noexcept = 0;
};
} // namespace oblivion
