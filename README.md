# OBLIVION

**Omnipotent Behavioral Learning & Intelligent Vision for Integrated Optimization & Navigation**

> **Beyond reaction. Beyond limits.**

OBLIVION is an embedded-first autonomous robotics intelligence platform. Its first target is competitive line-following robotics, with two shared-core operating profiles:

- **Sprint** — high-speed line tracking, predictive steering, trajectory and velocity optimization.
- **Explore** — track discovery, graph mapping, route memory, dead-end handling and mission-aware navigation.

The project is designed to grow into a reusable robotics platform. The initial 3D simulation workflow is documented in [simulator/README.md](simulator/README.md). The interactive viewer is published at [OBLIVION 3D Simulation Lab](https://jadhavdurvesh.github.io/OBLIVION/). It is not a dashboard and it is not a single PID sketch: the deliverable is the software that perceives sensor data, estimates robot state, plans actions and controls actuators on the robot.

## Engineering principles

1. **Deterministic control first.** Motor control and safety-critical work must have bounded execution time and must never wait for mapping, telemetry, model training or network activity.
2. **Algorithms must be real.** Every advertised controller or intelligence feature must have an implementation, tests, documented assumptions and benchmark evidence.
3. **Hardware-independent core.** Platform-specific code belongs behind narrow hardware abstraction interfaces.
4. **Measured complexity.** Additional languages and advanced algorithms are used only where they provide a concrete benefit. More languages do not automatically make firmware faster or safer.
5. **Fail safely.** Invalid sensor data, missed deadlines, numerical faults and actuator-limit violations must lead to a bounded safe response.
6. **Simulation before deployment.** Algorithms are tested against repeatable virtual tracks and disturbances before physical robot trials.

## Initial implementation status

The first milestone establishes a portable C++17 foundation:

- Differential-drive kinematic model.
- Calibrated reflectance perception with weighted line-position, width and confidence estimates.
- Shared robot, sensor and actuator data types.
- Controller interface and an advanced controller laboratory: PID, LQR runtime feedback, finite-horizon MPC prototype, sliding-mode control, ADRC prototype, backstepping feedback, adaptive PD, gain scheduling, fuzzy feedback and robust feedback prototype.
- Parallel controller bank with persistent worker threads, common output-polarity normalization, deadline monitoring, candidate telemetry and disagreement rejection.
- Sensor-to-motor control runtime joining reflectance perception, line-rate estimation, concurrent controller consensus and bounded differential motor requests; fails closed on stale/invalid frames, low battery, line loss, deadline misses and controller disagreement.
- Versioned CRC-16 framed robot protocol codec for future serial/Wi-Fi transport integration.
- Controller supervisor with explicit selection and fail-closed invalid-output handling.
- Motor-command safety limiting.
- Advanced controller laboratory: LQR, MPC, SMC, ADRC, backstepping, adaptive PD, gain scheduling, fuzzy feedback, robust feedback and H-infinity runtime realization.
- Offline H-infinity synthesis utility (`python/control/synthesize_hinfinity.py`).
- Deterministic differential-drive plant simulator with motor lag, acceleration and traction parameters.
- Initial 6-DOF rigid-body 3D plant with quaternion attitude, four-corner suspension/contact approximation, wheel rotational dynamics, tire slip/friction saturation, gravity, drag and motor torque response.
- Native 3D trace exporter and interactive Three.js viewer for robot pose playback, camera orbit, track scenarios, telemetry and trajectory inspection.
- Planar force/yaw dynamics prototype with gravity, slope, banking, inertia and friction limits.
- Curvature-aware motion planner with wheel-speed allocation, turn anticipation and forward/backward speed profiling.
- Parametric track builder for straights, circular arcs, S-bends and hairpins, with fork/intersection/decoy branch geometry and configurable line gaps.
- Deterministic reflectance-array simulation with seeded noise and sensor sample-dropout fault injection.
- Closed-loop simulation fails safe on invalid sensor samples and reports sensor faults.
- Closed-loop autonomous track runner integrating trajectory planning, reflectance perception, PID steering, motor limiting and the differential-drive plant.
- Integration tests for straight-track completion, controller injection, sensor-fault safe stops, complex track geometry and exact gap boundaries.
- Controller-injection benchmark executable comparing PID, LQR, sliding-mode and MPC on the same seeded S-bend scenario.
- Runnable straight-track simulation demo (host simulation only; not physical-robot validation).
- Dependency-free unit tests for kinematics, perception, PID and actuator limiting, plus a small executable example.

This foundation is not yet a competition-ready robot. The closed-loop track runner is an initial simulation integration, not a validated high-speed competition controller. H-infinity synthesis, constrained MPC, complete nonlinear plant validation, neural/RL policy training, state estimation, mapping, target-specific drivers and physical validation remain tracked work. See the master roadmap for the full feature register and exact completion gates.

## Repository layout

```text
include/oblivion/   Public, hardware-independent C++ interfaces
src/                Portable core implementation
tests/              Dependency-free unit tests
examples/           Executable demonstrations
docs/               Architecture, safety and development plans
python/             Offline simulation, analysis and training tools (future milestone)
platforms/          MCU-specific adapters (added with target hardware)
```

## Build and test

Requirements: CMake 3.16+ and a C++17 compiler (GCC, Clang or MSVC).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
./build/oblivion_demo
./build/oblivion_track_demo
./build/oblivion_controller_benchmark
./build/oblivion_3d_trace
```

On Windows, the demos may be located under `build/Release/`, depending on the generator. The controller benchmark prints CSV-style run metrics for repeatable comparisons.

## Language strategy

OBLIVION is intentionally polyglot only where justified:

| Language | Intended responsibility | Why |
|---|---|---|
| C++17 | Portable real-time core, control and navigation | Predictable performance, low-level control and broad MCU support |
| C | Vendor-specific MCU startup, peripheral and interrupt glue when required | Direct compatibility with many embedded SDKs and C ABIs |
| Python | Offline simulation, experiment orchestration, plotting, parameter search and ML training | Fast iteration and mature scientific/ML ecosystem |
| Rust (later, selective) | Optional host-side tooling or isolated safety-sensitive utilities if profiling and integration justify it | Strong memory-safety guarantees; not a reason to rewrite the real-time core |
| CMake / shell / PowerShell | Build and development automation | Reproducible builds across developer environments |

We will not split the 1 kHz control loop across languages or add assembly/Rust merely for complexity. Hot paths will be optimized only after profiling on a real target.

## Project identity

- Name: **OBLIVION**
- Expansion: **Omnipotent Behavioral Learning & Intelligent Vision for Integrated Optimization & Navigation**
- Project rights notice: see [LICENSE](LICENSE) and [NOTICE](NOTICE).

## Development status

See [live control runtime notes](docs/LIVE_CONTROL_RUNTIME.md), [docs/ROADMAP.md](docs/ROADMAP.md) for staged milestones and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the system design.

## Contributions and permissions

This is a proprietary project. The repository's custom license governs access and use. Do not assume that public visibility, source availability or a GitHub fork grants permission to use, modify, redistribute or commercially exploit the software.
