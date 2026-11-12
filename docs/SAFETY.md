# Safety and Reliability Requirements

OBLIVION is experimental robotics software. It is not certified for
safety-critical or life-safety applications.

## Mandatory invariants
- Never send NaN or infinity to an actuator adapter.
- Enforce configured motor command, acceleration and slew-rate limits.
- Reject stale sensor frames and invalid time deltas.
- Detect missed control deadlines.
- Keep emergency-stop handling independent of navigation and learning.
- Do not let telemetry, storage, networking or model inference block motor control.
- Default to zero/disabled actuator output after initialization or unrecoverable fault.
- Require explicit calibration and validated configuration before motion is enabled.

## Before physical testing
Use a raised-wheel test first, then low-speed tests in a clear, bounded area.
Provide a physical power disconnect and emergency stop. Verify motor polarity,
encoder direction, sensor calibration, watchdog operation and battery limits.
Increase speed only after repeatable tests demonstrate adequate tracking margin.

Software command limits cannot guarantee physical stopping distance. Braking
performance, traction, inertia and power electronics must be measured on the
actual robot.
