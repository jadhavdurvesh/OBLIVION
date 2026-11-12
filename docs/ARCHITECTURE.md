# OBLIVION Architecture

## Runtime boundary
The portable core owns algorithms and data contracts. Hardware-specific drivers
belong in platform adapters and must not leak vendor SDK types into core headers.

## Data flow
1. Hardware adapters acquire timestamped sensor samples.
2. Perception converts raw reflectance readings into line observations.
3. State estimation combines encoder and inertial data into a pose estimate.
4. Mission/navigation logic selects a desired path or local target.
5. Motion planning produces bounded velocity and curvature references.
6. Candidate controllers compute steering or wheel-speed requests.
7. The control supervisor validates timing, confidence and constraints.
8. The safety layer limits or rejects actuator commands.
9. The motor adapter applies the final command.

## Control timing
The motor-control path must be deterministic and independent of telemetry,
mapping persistence, network services, model training and other non-critical
work. The eventual control period is hardware-specific. Deadline misses must
be measured and surfaced.

## Differential-drive model
For left and right wheel linear velocities v_l and v_r, and wheel separation b:
- v = (v_l + v_r) / 2
- omega = (v_r - v_l) / b
- v_l = v - omega * b / 2
- v_r = v + omega * b / 2

Pose integration currently uses midpoint-heading integration. This is a
kinematic model, not a calibrated dynamic model; wheel slip, motor lag,
saturation, backlash and surface effects are not yet represented.

## Controller strategy
Controllers implement a common interface and are independently testable.
Planned implementations include PID, LQR, MPC, sliding-mode control, H-infinity,
ADRC, backstepping, adaptive control, gain scheduling and fuzzy control.
Neural and reinforcement-learning components are research/optimization
components until validated.

The supervisor must not blindly average controller outputs. It must use
explicit selection or mathematically justified blending, enforce actuator and
rate constraints, monitor numerical validity and provide a deterministic
fallback. Stability claims require analysis for the actual plant and switching
logic.

## Language boundaries
- C++17: portable embedded core and real-time algorithms.
- C: vendor SDK and low-level peripheral glue where required.
- Python: offline simulation, data analysis, tuning and ML training.
- Rust: optional host-side utilities or isolated components only when a
  demonstrated safety or maintenance benefit justifies FFI and build cost.

No language is introduced merely to increase the language count. The hot path
must not depend on Python, network services or dynamic allocation.

## Mapping and localization
Explore mode uses a topological graph of intersections and connecting edges.
Metric coordinates are estimates and must carry uncertainty. A reflectance
array alone does not provide global position. Loop closure requires reliable
landmarks or distinctive track observations, with encoder/IMU odometry treated
as drift-prone.

## Safety
All actuator outputs must be finite and bounded. Platform adapters must define
safe startup, watchdog behaviour, emergency stop behaviour, battery limits and
failure handling. Software limits are not a substitute for hardware interlocks
or physical testing.
