#include "oblivion/control/advanced_controllers.hpp"
#include "oblivion/control/pid_controller.hpp"
#include "oblivion/simulation/autonomous_track_runner.hpp"
#include <array>
#include <iomanip>
#include <iostream>

namespace {
oblivion::TrackRunnerConfig make_config() {
    using namespace oblivion;
    TrackRunnerConfig c{};
    c.plant.maximum_wheel_speed_mps = 0.6;
    c.plant.maximum_wheel_acceleration_mps2 = 4.0;
    c.plant.motor_time_constant_seconds = 0.02;
    c.motion.maximum_speed_mps = 0.32;
    c.motion.maximum_wheel_speed_mps = 0.6;
    c.motion.maximum_longitudinal_acceleration_mps2 = 0.8;
    c.motion.maximum_braking_deceleration_mps2 = 1.2;
    c.motion.maximum_lateral_acceleration_mps2 = 0.8;
    c.motion.maximum_yaw_rate_radps = 4.0;
    c.motion.wheel_track_m = c.plant.track_width_m;
    c.simulated_sensors.sensor_count = 7;
    c.perception.sensor_count = 7;
    for (std::size_t i = 0; i < 7; ++i) {
        const double lateral = (static_cast<double>(i) - 3.0) * 0.01;
        c.simulated_sensors.lateral_positions_m[i] = lateral;
        c.perception.positions_m[i] = lateral;
        c.perception.dark_level[i] = 0.0;
        c.perception.light_level[i] = 1.0;
    }
    c.perception.minimum_total_strength = 0.5;
    c.steering_gains = {8.0, 0.0, 0.05};
    c.steering_limits = {-0.5, 0.5, -0.1, 0.1};
    c.sensor_forward_offset_m = 0.035;
    c.completion_radius_m = 0.05;
    c.maximum_run_time_seconds = 15.0;
    return c;
}
}

int main() {
    using namespace oblivion;
    TrackBuilder builder(0.012);
    if (!builder.add_s_bend({0.0, 0.0}, 1.2, 0.10, 60)) return 1;
    const Track track = builder.build();
    if (!track.valid()) return 2;

    PidController pid({8.0, 0.0, 0.05}, {-0.5, 0.5, -0.1, 0.1});
    LqrController lqr({8.0, 0.5}, {-0.5, 0.5});
    SlidingModeController smc({3.0, 0.8, 0.05, 0.2, {-0.5, 0.5}});
    MpcConfig mpc_config{};
    mpc_config.limits = {-0.5, 0.5};
    MpcController mpc(mpc_config);

    struct Candidate { const char* name; IController* controller; };
    const std::array<Candidate, 4> candidates{{
        {"PID", &pid}, {"LQR", &lqr}, {"SMC", &smc}, {"MPC", &mpc}
    }};

    std::cout << "OBLIVION controller benchmark — identical S-bend scenario\n";
    std::cout << "Controller,Completed,Simulated_s,Max_error_m,RMS_error_m,Lost_steps\n";
    for (const auto& candidate : candidates) {
        auto config = make_config();
        AutonomousTrackRunner runner(config, *candidate.controller);
        const auto result = runner.run(track, {0.0, 0.0, 0.0}, 0.002, 7500);
        std::cout << std::fixed << std::setprecision(5)
                  << candidate.name << ','
                  << (result.completed ? "true" : "false") << ','
                  << result.simulated_time_seconds << ','
                  << result.maximum_cross_track_error_m << ','
                  << result.rms_cross_track_error_m << ','
                  << result.lost_line_steps << '\n';
        if (!result.valid) return 3;
    }
    return 0;
}
