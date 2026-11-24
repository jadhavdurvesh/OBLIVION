#include <exception>
#include <iostream>
void test_kinematics();
void test_embedded_odometry_slew();
void test_embedded_odometry_slew();
void test_robot_control_runtime();
void test_protocol_frame_codec();
void test_telemetry_codec();
void test_parallel_controller_bank();
void test_pid();
void test_perception();
void test_safety();
void test_advanced_controllers();
void test_hinfinity();
void test_simulation();
void test_rigid_body_3d();
void test_track_simulator();
void test_autonomous_track_runner();
void test_dynamics();
void test_motion_planner();
int main() {
    try { test_embedded_odometry_slew(); test_robot_control_runtime(); test_protocol_frame_codec(); test_telemetry_codec(); test_parallel_controller_bank(); test_kinematics(); test_perception(); test_pid(); test_safety(); test_advanced_controllers(); test_hinfinity(); test_simulation(); test_rigid_body_3d(); test_track_simulator(); test_autonomous_track_runner(); test_dynamics(); test_motion_planner(); std::cout << "All OBLIVION tests passed.\n"; return 0; }
    catch (const std::exception& e) { std::cerr << "TEST FAILURE: " << e.what() << '\n'; return 1; }
}
