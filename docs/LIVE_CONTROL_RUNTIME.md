LIVE CONTROL RUNTIME

This module is the first executable sensor-to-actuator control path for OBLIVION. It is not a hardware driver and has not been physically validated.

PARALLEL CONTROLLER BANK

ParallelControllerBank owns one persistent worker thread per registered controller. Each evaluation publishes the same input to every worker, waits up to a configured deadline, records candidates, normalizes output polarity, and computes median consensus only when enough controllers return valid values and candidate spread is within the configured disagreement bound.

Controller objects must outlive the bank. Do not call evaluate concurrently from multiple threads. A timed-out cycle is rejected; another evaluation is rejected until all workers finish the previous cycle. Each ControllerSlot has output_polarity +1 or -1 to map native controller sign to the shared steering convention. Calibrate this sign against the robot before enabling motors.

Median consensus is an initial arbitration policy, not a stability proof. Disagreement, deadline misses and insufficient valid controllers cause a zero command. Blending and automatic controller selection require separate validation.

SENSOR-TO-MOTOR RUNTIME

RobotControlRuntime validates frame timestamp, age, reflectance data and battery voltage; estimates line position and rate; evaluates the controller bank; rejects faults; converts consensus to differential motor requests; and applies the configured amplitude limiter. The first valid frame initializes the derivative and intentionally returns zero. Invalid frames, stale timestamps, invalid time deltas, lost line, low battery and controller faults return zero commands.

Commands are normalized to [-1,1]. Sensor sign and left/right motor mapping must be calibrated. MotorLimiter currently provides amplitude limiting only. Target hardware must add measured slew, acceleration, current, voltage, thermal and braking constraints.

WIRE PROTOCOL

protocol/frame_codec provides a versioned, length-bounded frame codec with sequence number and CRC-16/CCITT. Message types are reserved for sensor frames, motor commands, heartbeat and emergency stop. Payload schemas, serial/Wi-Fi transports, reconnect policy and target MCU firmware remain integration work.

DESKTOP AND EMBEDDED SPLIT

The threaded controller bank is a desktop execution backend, not for direct use in a bare-metal MCU control loop. The embedded runtime needs a statically scheduled, bounded execution model with measured worst-case execution time and independent hardware watchdog and emergency-stop path. Do not use an unvalidated network link as the sole real-time safety mechanism.

Physical motion must remain disabled until hardware protocol, polarity, sensor calibration, emergency stop and motor safety have been verified on the selected target.
