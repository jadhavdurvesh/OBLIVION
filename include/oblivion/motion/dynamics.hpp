#pragma once
#include <cstddef>

namespace oblivion {

// SI units throughout. Positive grade is uphill. Positive bank is banked
// toward the inside of a left-hand turn.
struct DynamicsParameters {
    double mass_kg{0.45};
    double yaw_inertia_kg_m2{0.0025};
    double wheelbase_track_m{0.12};
    double center_of_mass_height_m{0.035};
    double gravity_mps2{9.80665};
    double friction_coefficient{0.75};
    double rolling_resistance_coefficient{0.015};
    double linear_drag_coefficient{0.02};
    double angular_damping_n_m_per_radps{0.0005};
};

struct DynamicsState {
    double x_m{0.0};
    double y_m{0.0};
    double heading_rad{0.0};
    double forward_speed_mps{0.0};
    double lateral_speed_mps{0.0};
    double yaw_rate_radps{0.0};
    double longitudinal_acceleration_mps2{0.0};
    double lateral_acceleration_mps2{0.0};
};

struct DynamicsInput {
    double left_wheel_force_n{0.0};
    double right_wheel_force_n{0.0};
    double road_grade_rad{0.0};
    double road_bank_rad{0.0};
};

struct DynamicsResult {
    DynamicsState state{};
    double normal_load_n{0.0};
    double available_longitudinal_force_n{0.0};
    double requested_longitudinal_force_n{0.0};
    double applied_longitudinal_force_n{0.0};
    double required_lateral_force_n{0.0};
    double traction_utilization{0.0};
    bool traction_limited{false};
    bool valid{false};
};

class DifferentialDriveDynamics {
public:
    explicit DifferentialDriveDynamics(DynamicsParameters parameters) noexcept;
    [[nodiscard]] bool parameters_valid() const noexcept;
    [[nodiscard]] DynamicsResult step(const DynamicsState& state,
                                      const DynamicsInput& input,
                                      double dt_seconds) const noexcept;
    [[nodiscard]] const DynamicsParameters& parameters() const noexcept;
private:
    DynamicsParameters parameters_;
};

} // namespace oblivion
