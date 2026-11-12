#include "oblivion/simulation/autonomous_track_runner.hpp"
#include <iomanip>
#include <iostream>

int main() {
    using namespace oblivion;
    TrackBuilder builder(0.012);
    if (!builder.add_straight({0.0, 0.0}, {1.0, 0.0})) return 1;

    TrackRunnerConfig config{};
    config.plant.maximum_wheel_speed_mps = 0.7;
    config.plant.maximum_wheel_acceleration_mps2 = 4.0;
    config.plant.motor_time_constant_seconds = 0.015;
    config.motion.maximum_speed_mps = 0.55;
    config.motion.maximum_wheel_speed_mps = 0.7;
    config.motion.maximum_longitudinal_acceleration_mps2 = 1.0;
    config.motion.maximum_braking_deceleration_mps2 = 1.2;
    config.motion.maximum_lateral_acceleration_mps2 = 1.5;
    config.motion.maximum_yaw_rate_radps = 5.0;
    config.motion.wheel_track_m = config.plant.track_width_m;
    config.simulated_sensors.sensor_count = 5;
    config.perception.sensor_count = 5;
    for (std::size_t i = 0; i < 5; ++i) {
        const double lateral = (static_cast<double>(i) - 2.0) * 0.01;
        config.simulated_sensors.lateral_positions_m[i] = lateral;
        config.perception.positions_m[i] = lateral;
        config.perception.dark_level[i] = 0.0;
        config.perception.light_level[i] = 1.0;
    }
    config.perception.minimum_total_strength = 0.5;
    config.steering_gains = {8.0, 0.0, 0.05};
    config.steering_limits = {-0.5, 0.5, -0.1, 0.1};
    config.sensor_forward_offset_m = 0.025;
    config.completion_radius_m = 0.045;
    config.maximum_run_time_seconds = 8.0;

    AutonomousTrackRunner runner(config);
    const auto result = runner.run(builder.build(), {0.0, 0.0, 0.0}, 0.005, 1600);
    if (!result.valid) {
        std::cerr << "Simulation failed: invalid runner result or configuration.\n";
        return 2;
    }

    std::cout << std::fixed << std::setprecision(4)
              << "OBLIVION closed-loop track simulation\n"
              << "completed=" << (result.completed ? "true" : "false") << '\n'
              << "simulated_time_s=" << result.simulated_time_seconds << '\n'
              << "estimated_plan_time_s=" << result.estimated_time_seconds << '\n'
              << "steps=" << result.steps << '\n'
              << "max_cross_track_error_m=" << result.maximum_cross_track_error_m << '\n'
              << "rms_cross_track_error_m=" << result.rms_cross_track_error_m << '\n'
              << "lost_line_steps=" << result.lost_line_steps << '\n'
              << "final_x_m=" << result.final_pose.x_m << '\n'
              << "final_y_m=" << result.final_pose.y_m << '\n';
    return result.completed ? 0 : 3;
}
