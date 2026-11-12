# OBLIVION Roadmap

## M0 — Foundation (in progress)
- [x] Repository identity and proprietary notice
- [x] Portable C++17 build
- [x] Differential-drive kinematic model
- [x] Controller contract and bounded PID baseline
- [x] Motor command limiter
- [x] Dependency-free unit tests and demo
- [ ] CI build matrix (GCC, Clang, MSVC)
- [ ] Static analysis and formatting checks

## M1 — Perception contracts
- [ ] Reflectance calibration model
- [ ] Line observation and confidence model
- [ ] Weighted line position and width estimation
- [ ] Junction/curve/lost-line feature extraction
- [ ] Synthetic sensor test fixtures

## M2 — State estimation
- [ ] Encoder odometry
- [ ] IMU integration and bias handling
- [ ] Sensor timestamp and stale-data policy
- [ ] EKF implementation and consistency tests
- [ ] Localization uncertainty representation

## M3 — Control laboratory
- [x] PID baseline
- [ ] LQR
- [ ] MPC
- [ ] Sliding-mode control
- [ ] H-infinity design/evaluation
- [ ] ADRC
- [ ] Backstepping
- [ ] Adaptive control and gain scheduling
- [ ] Fuzzy control
- [ ] Controller supervisor and safe transitions
- [ ] Stability, timing and resource benchmarks

## M4 — Motion intelligence
- [ ] Curvature estimation
- [ ] Velocity and acceleration profiles
- [ ] Jerk and wheel-speed constraints
- [ ] Braking-distance planner
- [ ] Motor identification and slip indicators

## M5 — Explore navigation
- [ ] Track graph data model
- [ ] Intersection identity and loop closure
- [ ] Edge confidence and traversal memory
- [ ] DFS/BFS/Dijkstra/A* planners
- [ ] Dead-end and mission-specific decoy classification
- [ ] Safe backtracking and replanning

## M6 — Learning and optimization
- [ ] Python offline simulation harness
- [ ] Repeatable track and disturbance scenarios
- [ ] Parameter optimization
- [ ] Neural prediction experiments
- [ ] Offline reinforcement-learning experiments
- [ ] Model/policy validation and deployment format

## M7 — Hardware integration
- [ ] Select MCU and sensor/motor hardware
- [ ] Implement target-specific HAL adapters
- [ ] Measure control-loop WCET and jitter
- [ ] Hardware-in-the-loop tests
- [ ] Physical track validation and tuning

A feature is complete only when implementation, tests, limitations and benchmark
results are documented.
