cmake_minimum_required(VERSION 3.16)
project(OBLIVION VERSION 0.1.0 LANGUAGES CXX)

option(OBLIVION_BUILD_TESTS "Build OBLIVION unit tests" ON)
option(OBLIVION_BUILD_EXAMPLES "Build OBLIVION examples" ON)

add_library(oblivion_core
    src/kinematics/differential_drive.cpp
    src/perception/reflectance_processor.cpp
    src/control/pid_controller.cpp
    src/control/advanced_controllers.cpp
    src/control/h_infinity_controller.cpp
    src/safety/motor_limiter.cpp
    src/simulation/differential_drive_plant.cpp
    src/simulation/track_simulator.cpp
    src/simulation/autonomous_track_runner.cpp
    src/motion/dynamics.cpp
    src/motion/planner.cpp
)
target_include_directories(oblivion_core PUBLIC include)
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
    target_link_libraries(oblivion_track_demo PRIVATE oblivion_core)
    target_link_libraries(oblivion_demo PRIVATE oblivion_core)
endif()

if(OBLIVION_BUILD_TESTS)
    enable_testing()
    add_executable(oblivion_tests
        tests/test_main.cpp
        tests/test_kinematics.cpp
        tests/test_perception.cpp
        tests/test_pid.cpp
        tests/test_advanced_controllers.cpp
        tests/test_hinfinity.cpp
        tests/test_simulation.cpp
        tests/test_track_simulator.cpp
        tests/test_autonomous_track_runner.cpp
        tests/test_dynamics.cpp
        tests/test_motion_planner.cpp
        tests/test_safety.cpp
    )
    target_link_libraries(oblivion_tests PRIVATE oblivion_core)
    add_test(NAME oblivion_tests COMMAND oblivion_tests)
endif()
