# OBLIVION

**Omnipotent Behavioral Learning & Intelligent Vision for Integrated Optimization & Navigation**

> **Beyond reaction. Beyond limits.**

OBLIVION is an embedded-first autonomous robotics intelligence platform. Its first target is competitive line-following robotics, with two shared-core operating profiles:

- **Sprint** — high-speed line tracking, predictive steering, trajectory and velocity optimization.
- **Explore** — track discovery, graph mapping, route memory, dead-end handling and mission-aware navigation.

The project is designed to grow into a reusable robotics platform. It is not a dashboard and it is not a single PID sketch: the deliverable is the software that perceives sensor data, estimates robot state, plans actions and controls actuators on the robot.

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
- Shared robot, sensor and actuator data types.
- Controller interface and a baseline PID implementation.
- Motor-command safety limiting.
- Dependency-free unit tests and a small executable example.

This foundation is not yet a competition-ready robot. Hardware drivers, calibrated dynamics, localization, perception, mapping, advanced controllers and physical validation are subsequent milestones.

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
```

On Windows, the demo may be located at `build/Release/oblivion_demo.exe`, depending on the generator.

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

See [docs/ROADMAP.md](docs/ROADMAP.md) for staged milestones and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the system design.

## Contributions and permissions

This is a proprietary project. The repository's custom license governs access and use. Do not assume that public visibility, source availability or a GitHub fork grants permission to use, modify, redistribute or commercially exploit the software.
