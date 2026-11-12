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

The deterministic host-side plant simulator adds first-order motor lag, wheel
acceleration limits and a traction factor. A separate planar force/yaw model
includes mass, yaw inertia, grade, banking, rolling resistance, drag and a
simplified per-wheel friction budget. The autonomous track runner connects
parametric track geometry, synthetic reflectance sensors, line perception,
trajectory planning, planner-derived left/right wheel-speed feedforward,
pluggable controller feedback, motor limiting and the differential-drive plant.
PID is the default; alternative controller instances can be injected so that
multiple control laws can be evaluated against the same deterministic track.
It reports tracking error, line-loss counts, simulated time and completion.
Track paths are resampled before planning so sparse geometric input does not
create discontinuous speed commands.

This is an initial host-side simulation and closed-loop integration, not a
validated digital twin or competition-ready controller. The current runner
uses the selected controller for line-position correction and a nearest-point reference; it does not yet implement
full pure-pursuit preview, MPC path tracking, dynamic load transfer, wheel
rotational inertia, calibrated motor electrical dynamics or tire relaxation.
All results are simulation evidence only.


## Motion mathematics and physical modelling

The software-only motion layer uses SI units and explicitly separates a
kinematic model from a force-based planar dynamics approximation.

### Differential-drive geometry
For wheel track width b, center speed v and signed path curvature kappa:
- yaw rate: omega = v * kappa
- left wheel speed: v_left = v * (1 - kappa*b/2)
- right wheel speed: v_right = v * (1 + kappa*b/2)
- signed curvature from three points: kappa = 2*cross(B-A,C-A) /
  (|AB|*|BC|*|CA|)

Positive curvature denotes a left turn. The inner wheel is therefore the
left wheel for a left turn and the right wheel for a right turn.

### Turn anticipation and braking
The initial turn-anticipation estimate is:
d = v*t_latency + v^2/(2*a_brake) + d_margin.
This is a planning estimate, not a physical guarantee; measured actuator delay,
jerk, tire friction, grade and uncertainty must eventually refine it.

A backward speed pass enforces v_i^2 <= v_(i+1)^2 + 2*a_brake*ds.
A forward pass enforces acceleration feasibility. Curvature limits speed using
a_lat = v^2*|kappa|, yaw-rate limits and the maximum outer-wheel speed.

### Planar force and yaw model
The software dynamics model includes mass m, yaw inertia I_z, wheel separation b,
wheel forces, rolling resistance, quadratic drag, grade and bank:
- normal load approximation: N = m*g*cos(grade)*cos(bank)
- longitudinal grade force: F_grade = m*g*sin(grade)
- yaw torque: tau_z = b*(F_right-F_left)/2 - c_omega*omega
- yaw acceleration: alpha_z = tau_z/I_z
- lateral demand: F_y = m*(v*omega - g*sin(bank)*cos(grade))
- available tire force is bounded by a friction-circle approximation.

The simulator limits each wheel's longitudinal force using its share of the
available friction budget. This is a simplified planar model: it does not yet
model load transfer, tire relaxation, wheel rotational inertia, drivetrain
compliance, motor electrical dynamics, or full 3D roll/pitch dynamics.

### Physical correctness boundaries
Gravity on a level track acts vertically and does not independently create
sideways motion. It affects normal load and therefore traction; on a slope its
component along the surface affects acceleration and braking. Banking can
supply part of the centripetal force. The model must not double-count that
gravity component.

All parameters are configurable and use SI units. Defaults are placeholders,
not measured specifications for a physical OBLIVION robot.

## Controller strategy
Controllers implement a common interface and are independently testable.
The current controller laboratory includes PID, LQR runtime feedback with precomputed gains, a finite-horizon MPC prototype, sliding-mode control, ADRC, backstepping, adaptive PD, gain scheduling, fuzzy feedback and a bounded robust-feedback prototype. These are at different maturity levels; see the master roadmap. In particular, the robust-feedback prototype is not H-infinity synthesis, and a production LQR requires plant-specific offline Riccati design and validation. MPC now has a deterministic three-segment predictive prototype with bounded actuator, acceleration, rate and slew-cost handling; it is not yet a full constrained multivariable optimizer. An offline Python bounded-real-LMI H-infinity synthesis utility and a fixed-memory runtime gain realization are present; generated gains are only as valid as the identified plant and solver result. Neural inference and reinforcement-learning policies remain explicit tracked work. Neural and reinforcement-learning components are research/optimization components until validated.

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
