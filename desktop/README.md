OBLIVION DESKTOP CONTROL CENTER

The native desktop application is a Qt 6 Widgets application using Qt SerialPort. It is built as an optional target when Qt 6 Widgets and SerialPort development packages are installed.

BUILD

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DOBLIVION_BUILD_DESKTOP=ON
cmake --build build --config Release

If Qt 6 is not found, the portable core and tests still build; CMake reports that the desktop target was skipped. Install Qt 6 Widgets and SerialPort development components and configure again to build the desktop app.

CURRENT UI ACTIONS

- Refresh available serial ports.
- Connect/disconnect at 115200 baud.
- Select Sprint or Explore mission profile.
- Select multiple controllers at once using the parallel controller ensemble checkboxes.
- Send a 16-bit controller selection mask and Sprint/Explore mission profile with command frames.
- Select a recommended controller ensemble or clear the selection.
- Send Start, Pause, Stop, Emergency Stop, Heartbeat and Calibration command frames.
- Display received framed packets, sequence numbers, protocol errors and serial errors.
- Clear the event log.

WIRE FORMAT

The frame codec is version 1: magic bytes 0x4f 0x42, version byte, message type byte, little-endian uint32 sequence, little-endian uint16 payload length, payload (maximum 512 bytes), then little-endian CRC-16/CCITT over header and payload. Desktop command payload is currently three bytes: action code, mission profile index, controller index. MotorCommand action codes are 0=stop, 1=start and 2=pause. Calibration uses MessageType::Calibration and action code 0x10. Emergency stop uses its dedicated message type.

IMPORTANT FIRMWARE CONTRACT

The matching robot firmware is not yet implemented. The UI can open a serial port and transmit framed commands, but a connection is not proof that the robot understands or safely executes them. Firmware must implement the exact protocol, reject unsupported profile/controller IDs, return telemetry frames, enforce command watchdog timeout, default motors off at boot, and implement emergency stop in an independent hardware-safe path. A lost USB/serial link must never leave motors running. Do not connect live motors until this firmware contract and physical E-stop are verified.

The current receive path logs valid framed packet metadata; typed telemetry payload decoding and charts are future work. Desktop Start/Pause/Stop currently send requests; robot state is not assumed until an acknowledged status/telemetry protocol is added.


CONTROLLER ENSEMBLE NOTE

The desktop selection is a multi-controller request, not a claim that the desktop independently runs real-time motor controllers. The embedded controller bank is responsible for executing the selected controllers, enforcing per-controller deadlines, validating outputs and applying the configured supervisor/arbitration policy. The desktop command payload now contains: action byte, mission profile byte (0=Sprint, 1=Explore), controller mask low byte, controller mask high byte. This is a protocol payload change from the earlier single-controller-index draft. Firmware must implement this exact payload before using the redesigned UI with a physical robot. Do not assume the robot is running every selected controller until firmware reports an acknowledged active-controller mask.
