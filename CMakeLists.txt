cmake_minimum_required(VERSION 3.16)
project(OBLIVION VERSION 0.2.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

option(OBLIVION_BUILD_TESTS "Build OBLIVION unit tests" ON)
option(OBLIVION_BUILD_EXAMPLES "Build OBLIVION examples" ON)

add_library(oblivion_core
    src/kinematics/differential_drive.cpp
    src/perception/reflectance_processor.cpp
    src/control/pid_controller.cpp
    src/control/advanced_controllers.cpp
    src/control/h_infinity_controller.cpp
    src/control/parallel_controller_bank.cpp
    src/control/embedded_controller_bank.cpp
    src/runtime/robot_control_runtime.cpp
    src/protocol/frame_codec.cpp
    src/protocol/telemetry.cpp
    src/safety/motor_limiter.cpp
    src/safety/motor_slew_limiter.cpp
    src/localization/odometry.cpp
    src/simulation/rigid_body_3d.cpp
    src/simulation/differential_drive_plant.cpp
    src/simulation/track_simulator.cpp
    src/simulation/autonomous_track_runner.cpp
    src/motion/dynamics.cpp
    src/motion/planner.cpp
)
target_include_directories(oblivion_core PUBLIC include)
target_link_libraries(oblivion_core PUBLIC Threads::Threads)
target_compile_features(oblivion_core PUBLIC cxx_std_17)
set_target_properties(oblivion_core PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(oblivion_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(oblivion_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(OBLIVION_BUILD_EXAMPLES)
    add_executable(oblivion_demo examples/core_demo.cpp)
    add_executable(oblivion_track_demo examples/track_runner_demo.cpp)
    add_executable(oblivion_3d_trace examples/rigid_body_3d_trace.cpp)
    target_link_libraries(oblivion_3d_trace PRIVATE oblivion_core)
    add_executable(oblivion_controller_benchmark examples/controller_benchmark.cpp)
    target_link_libraries(oblivion_controller_benchmark PRIVATE oblivion_core)
    target_link_libraries(oblivion_track_demo PRIVATE oblivion_core)
    target_link_libraries(oblivion_demo PRIVATE oblivion_core)
endif()

if(OBLIVION_BUILD_TESTS)
    enable_testing()
    add_executable(oblivion_tests
        tests/test_main.cpp
        tests/test_embedded_odometry_slew.cpp
        tests/test_parallel_controller_bank.cpp
        tests/test_protocol_frame_codec.cpp
        tests/test_telemetry_codec.cpp
        tests/test_robot_control_runtime.cpp
        tests/test_kinematics.cpp
        tests/test_perception.cpp
        tests/test_pid.cpp
        tests/test_advanced_controllers.cpp
        tests/test_hinfinity.cpp
        tests/test_rigid_body_3d.cpp
        tests/test_simulation.cpp
        tests/test_track_simulator.cpp
        tests/test_autonomous_track_runner.cpp
        tests/test_dynamics.cpp
        tests/test_motion_planner.cpp
        tests/test_safety.cpp
    )
    target_link_libraries(oblivion_tests PRIVATE oblivion_core)
    add_test(NAME oblivion_tests COMMAND oblivion_tests)
    if(OBLIVION_BUILD_EXAMPLES)
        add_test(NAME oblivion_controller_benchmark COMMAND oblivion_controller_benchmark)
        add_test(NAME oblivion_3d_trace_export
                 COMMAND oblivion_3d_trace ${CMAKE_CURRENT_BINARY_DIR}/oblivion_3d_trace.json)
    endif()
endif()

option(OBLIVION_BUILD_DESKTOP "Build the native Qt desktop control center when Qt 6 is available" ON)
if(OBLIVION_BUILD_DESKTOP)
    find_package(Qt6 QUIET COMPONENTS Widgets SerialPort Network)
    if(Qt6_FOUND)
        add_executable(oblivion_desktop desktop/src/main.cpp)
        target_link_libraries(oblivion_desktop PRIVATE oblivion_core Qt6::Widgets Qt6::SerialPort Qt6::Network)
        target_compile_features(oblivion_desktop PRIVATE cxx_std_17)
        set_target_properties(oblivion_desktop PROPERTIES WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE)
        message(STATUS "OBLIVION native desktop control center enabled")
    else()
        message(STATUS "Qt 6 Widgets/SerialPort not found; native desktop target skipped")
    endif()
endif()
