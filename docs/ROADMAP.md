# OBLIVION Master Roadmap and Feature Register

This is the authoritative feature inventory. Do not rely on chat history or
README summaries to decide whether a subsystem is planned. Every item remains
tracked here until its implementation, tests, documentation and target evidence
meet its exit criteria.

Status meanings:
- [x] Implemented in repository with tests; not automatically hardware-validated.
- [~] Partial foundation exists; further implementation or validation remains.
- [ ] Not yet implemented.
- [!] Requires offline design/training, target hardware, external evidence or a
      deliberate engineering decision before it can be considered complete.

## M0 — Repository, build and engineering foundation
- [x] Proprietary project identity, LICENSE and NOTICE.
- [x] Portable C++17 CMake project.
- [x] GCC, Clang and MSVC CI workflow (keep green on every main update).
- [x] Dependency-free test runner and executable demo.
- [ ] Static analysis: clang-tidy and compiler sanitizers on host builds.
- [ ] Formatting enforcement and include-what-you-use review.
- [ ] Reproducible release builds, versioning and artifact provenance.
- [ ] Software bill of materials and third-party license inventory.
- [ ] Signed release artifacts and release verification.

## M1 — Perception and sensor interpretation
- [x] Calibrated reflectance normalization, line polarity, weighted position,
      width estimate and confidence.
- [ ] Calibration acquisition workflow, outlier rejection and calibration storage.
- [ ] Sensor health, stuck-at detection, contamination and drift monitoring.
- [ ] Multi-line/ambiguous pattern detection and saturation handling.
- [ ] Junction, T-junction, crossroad, fork and marker classification.
- [ ] Curve, S-curve, hairpin and curvature estimation.
- [ ] Broken-line, gap, lost-line and reacquisition classification.
- [ ] Temporal perception filters and confidence/uncertainty propagation.
- [ ] Optional camera perception adapter and vision-based line segmentation.
- [ ] Sensor fusion for reflectance, camera, IMU and encoders.
- [ ] Synthetic sensor generation and perception benchmark corpus.

## M2 — State estimation and localization
- [ ] Encoder odometry with wheel geometry calibration.
- [ ] IMU bias estimation, integration and gravity/orientation handling.
- [ ] Timestamp synchronization, stale-data rejection and clock rollover policy.
- [ ] Complementary filter.
- [ ] Extended Kalman Filter (EKF), covariance and innovation gating.
- [ ] Unscented Kalman Filter (UKF) evaluation if nonlinearities justify it.
- [ ] Particle filter evaluation for ambiguous/global localization.
- [ ] Slip detection and odometry uncertainty.
- [ ] Track-relative localization and landmark/loop-closure support.
- [ ] State validity, covariance bounds and degraded-mode policy.

## M3 — Controller laboratory: runtime algorithms
Each controller is a candidate implementation, not a claim that it is optimal
for every robot. Controllers must be independently testable and evaluated on a
documented plant model before supervisor selection.

- [x] PID baseline with output/integral bounds, conditional anti-windup and
      derivative-on-measurement.
- [~] LQR runtime feedback using precomputed gains; offline Riccati synthesis,
      plant-specific Q/R selection and stability validation remain.
- [~] MPC deterministic three-segment finite-horizon predictive optimizer with bounded acceleration, rate, actuator and slew-cost constraints; full multivariable plant constraints, solver timing and model validation remain.
- [x] Sliding-mode control prototype with boundary-layer saturation.
- [~] ADRC observer/control prototype; plant-relative bandwidth tuning,
      noise robustness and discrete-time observer validation remain.
- [~] Backstepping feedback prototype; derive and validate the actual nonlinear
      plant model and Lyapunov conditions before deployment.
- [x] Adaptive PD gain update with bounded gains.
- [x] Gain-scheduled feedback with validated piecewise-linear interpolation.
- [x] Fuzzy feedback prototype; formalize membership functions/rule base and
      tune against benchmark data.
- [~] Robust feedback prototype with a bounded uncertainty correction; this is
      NOT a synthesized H-infinity controller.
- [~] Offline continuous-time H-infinity state-feedback synthesis script using a bounded-real LMI (requires identified plant matrices and external solver validation).
- [x] H-infinity runtime realization from supplied precomputed gains with finite-state envelope and actuator limits; hardware certification remains outstanding.
- [ ] Disturbance observer and explicit robust/adaptive comparison benchmarks.
- [ ] Bumpless transfer and state handover between controllers.
- [ ] Controller health, numerical-fault and deadline monitors.
- [ ] Supervisor policies: fixed selection, mode-based selection and validated
      fallback; no arbitrary averaging of incompatible outputs.
- [ ] Stability analysis for switching/blending policies and actuator saturation.
- [ ] Deterministic control benchmarks: WCET, jitter, RAM, flash and CPU load.

## M4 — Learning-based control and model intelligence
- [ ] Neural network inference interface with fixed-size, allocation-free runtime.
- [ ] Offline training pipeline and dataset/version provenance.
- [ ] Quantization, model export and MCU memory/latency profiling.
- [ ] Neural curvature, line-loss and motor-response predictors.
- [ ] Learned residual/model correction with bounded contribution.
- [ ] Reinforcement-learning environment and reward definition.
- [ ] Offline RL training and reproducible experiment tracking.
- [ ] Policy evaluation against classical baselines and adversarial scenarios.
- [ ] Safe policy envelope, runtime monitor and deterministic fallback.
- [ ] Policy signing/versioning and deployment compatibility checks.
- [ ] Online learning only after safety case and resource analysis; never
      unrestricted online motor-policy updates.

## M5 — Motion and trajectory intelligence
- [ ] Robot/motor dynamic model, actuator dead zone and response delay.
- [ ] Motor identification from measured step/ramp experiments.
- [~] Differential wheel-speed and outer-wheel-speed constraints (torque/current/voltage/thermal constraints remain).
- [x] Curvature-aware speed profile and lateral-acceleration limits (host planner prototype).
- [ ] Acceleration, deceleration and jerk-limited trajectory generation.
- [~] Braking-distance and turn-anticipation estimates (measured stopping data and emergency-stop validation remain).
- [ ] Wheel slip, traction and surface-change estimation.
- [ ] Battery-aware power and speed management.
- [ ] Time-optimal/energy-aware trajectory optimization with bounded compute.
- [ ] Run-to-run parameter optimization with rollback and validation.

## M6 — Sprint mission profile
- [ ] High-rate line tracking and predictive steering.
- [ ] Corner entry, apex and exit strategy.
- [ ] Straight-line acceleration and braking zones.
- [ ] Curvature preview and speed feasibility.
- [ ] Lap/sector timing and repeatable comparison.
- [ ] Track-specific speed profile learning.
- [ ] Sprint mode resource budget and minimal mapping overhead.
- [ ] Lost-line recovery and safe abort.

## M7 — Explore mission profile and cognitive mapping
- [ ] Track graph: nodes, edges, junction identity and traversal metadata.
- [ ] Branch classification and confidence.
- [ ] DFS and BFS exploration.
- [ ] Dijkstra and A* weighted route planning.
- [ ] Dead-end detection and safe backtracking.
- [ ] Loop detection, map correction and localization uncertainty.
- [ ] Decoy/ambiguous branch classification based on explicit track rules/evidence.
- [ ] Route memory, persistent map format and map integrity checks.
- [ ] Mission policies: explore all, shortest route, target branch and return home.
- [ ] Explore mode recovery, replanning and resource limits.

## M8 — Simulation, digital twin and verification
- [x] Deterministic differential-drive plant with motor lag, acceleration and traction factor.
- [x] Deterministic synthetic reflectance-array sampling against track geometry.
- [x] Closed-loop host simulation runner connecting track, sensors, perception, planned wheel-speed feedforward, PID correction, motor limiter and plant.
- [~] Straight-track end-to-end integration test and runnable demo; curved-track, noise, gap, disturbance and Monte Carlo validation remain.
- [ ] Motor lag, acceleration, braking, saturation and wheel-slip models.
- [~] Sensor geometry, deterministic noise and random sample dropout are implemented; drift, stuck-at, correlated failures and broader fault injection remain.
- [~] Parametric track generator: straights, circular arcs, S-bends, hairpins, branches and line gaps; intersection topology, loops and dead-end scenarios remain.
- [ ] Sprint and Explore benchmark suites.
- [~] Complex track geometry and sensor-dropout integration tests; curved-track closed-loop performance, disturbances and Monte Carlo robustness remain.
- [ ] Monte Carlo robustness and sensitivity tests.
- [ ] Controller comparison reports and reproducible experiment seeds.
- [ ] Property-based and fuzz testing of parsers, configuration and math boundaries.
- [ ] Hardware-in-the-loop interface and trace replay.
- [ ] Digital-twin versus physical-run model calibration.

## M9 — Embedded runtime and hardware abstraction
- [ ] HAL interfaces for clock, GPIO, ADC, PWM, encoders, IMU and battery.
- [ ] Safe motor-driver abstraction and hardware emergency-stop input.
- [ ] Startup calibration and actuator-disabled boot state.
- [ ] Watchdog, brownout and reset-cause handling.
- [ ] Deterministic task scheduler and deadline accounting.
- [ ] Lock-free or bounded communication between control and background tasks.
- [ ] No heap allocation in critical control path after initialization.
- [ ] Telemetry ring buffer and offline trace export.
- [ ] Platform adapters after MCU selection: Arduino-class, ESP32 and STM32
      only where the chosen board/SDK supports required timing and peripherals.
- [ ] Measure WCET, jitter, stack, RAM and flash on each actual target.

## M10 — Operations, security and reliability
- [ ] Typed configuration schema, range checks and safe defaults.
- [ ] Configuration integrity, versioning and rollback.
- [ ] Fault taxonomy, diagnostics and fault-injection tests.
- [ ] Brownout, sensor disconnect, motor stall and over-temperature handling.
- [ ] Communication protocol framing, authentication and replay protection if
      remote telemetry/control is introduced.
- [ ] Signed firmware/update packages if update support is introduced.
- [ ] Safety case and physical test checklist.
- [ ] Competition preflight, calibration and post-run reports.

## M11 — Language-specific tools (only where justified)
- [x] C++17 core as the portable embedded baseline.
- [ ] C platform glue for vendor SDKs/interrupts when a target requires it.
- [ ] Python simulation, analysis, parameter search and offline ML tooling.
- [ ] Rust host utilities or isolated components only where measured benefits
      outweigh FFI/build complexity.
- [ ] Assembly only for profiled bottlenecks with equivalent correctness tests.
- [ ] C ABI boundaries and cross-language test vectors where multiple languages
      exchange data.

 
## M12 — Software-only mathematical motion intelligence
- [x] Signed path curvature from three-point geometry.
- [x] Closed-loop straight-track simulation from generated track to robot motion and tracking metrics.
- [x] Planner-derived left/right wheel-speed feedforward with bounded line-position correction.
- [x] Sparse path resampling before speed-profile generation.
- [x] Differential wheel-speed allocation from linear speed and curvature.
- [x] Turn anticipation using latency, braking distance and safety margin.
- [x] Curvature, yaw-rate and outer-wheel-speed corner velocity limits.
- [x] Forward acceleration and backward braking speed-profile passes.
- [x] Grade-aware longitudinal acceleration and braking approximation.
- [x] Bank-aware lateral acceleration envelope (simplified planar model).
- [x] Planar force model with mass, yaw inertia, differential wheel force,
      rolling resistance, quadratic drag and gravity components.
- [x] Friction-circle-inspired per-wheel traction limiting prototype.
- [ ] Jerk-limited S-curve trajectory generation and continuous-time profiles.
- [ ] Load transfer, center-of-mass dynamics and wheel normal-load estimation.
- [ ] Wheel rotational inertia, motor torque-speed curve and drivetrain losses.
- [ ] Battery/electrical motor model including back-EMF, resistance and current.
- [ ] Tire relaxation, lateral slip, longitudinal slip and combined-slip model.
- [ ] Full 3D roll/pitch dynamics for uneven terrain (only if robot geometry warrants).
- [ ] Uncertainty-aware trajectory optimization and robust braking margins.
- [ ] Receding-horizon path tracking with obstacle and track-boundary constraints.
- [ ] Independent derivation review and numerical validation against reference cases.

## Completion gates
A feature is not complete because a file, class, interface, placeholder or
roadmap checkbox exists. Completion requires:
1. A documented mathematical/behavioural specification and assumptions.
2. A working implementation with bounded failure behaviour.
3. Unit, boundary, invalid-input and regression tests.
4. Integration tests with neighbouring subsystems.
5. Benchmarks and resource measurements appropriate to its target.
6. Known limitations and deployment constraints documented.
7. Hardware validation before claims of physical-robot readiness.

The Sprint and Explore profiles share the same core, but do not run expensive
mapping or learning work on the critical motor-control path.
