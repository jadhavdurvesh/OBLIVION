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
- Select one of the implemented controller identifiers.
- Send Start, Pause, Stop, Emergency Stop, Heartbeat and Calibration command frames.
- Display received framed packets, sequence numbers, protocol errors and serial errors.
- Clear the event log.

WIRE FORMAT

The frame codec is version 1: magic bytes 0x4f 0x42, version byte, message type byte, little-endian uint32 sequence, little-endian uint16 payload length, payload (maximum 512 bytes), then little-endian CRC-16/CCITT over header and payload. Desktop command payload is currently three bytes: action code, mission profile index, controller index. MotorCommand action codes are 0=stop, 1=start and 2=pause. Calibration uses MessageType::Calibration and action code 0x10. Emergency stop uses its dedicated message type.

IMPORTANT FIRMWARE CONTRACT

The matching robot firmware is not yet implemented. The UI can open a serial port and transmit framed commands, but a connection is not proof that the robot understands or safely executes them. Firmware must implement the exact protocol, reject unsupported profile/controller IDs, return telemetry frames, enforce command watchdog timeout, default motors off at boot, and implement emergency stop in an independent hardware-safe path. A lost USB/serial link must never leave motors running. Do not connect live motors until this firmware contract and physical E-stop are verified.

The current receive path logs valid framed packet metadata; typed telemetry payload decoding and charts are future work. Desktop Start/Pause/Stop currently send requests; robot state is not assumed until an acknowledged status/telemetry protocol is added.
