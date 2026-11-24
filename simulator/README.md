# OBLIVION 3D Simulation Lab

The 3D Lab pairs a native C++ rigid-body physics foundation with a browser-based Three.js trace viewer.

- Native C++ owns the simulation state, differential motor commands, wheel rotation, four-corner suspension/contact approximation, tire slip and friction limits, gravity, drag and quaternion attitude integration.
- The browser renders the robot, 3D track scenarios, trajectory trace, camera and telemetry. It does not replace native physics with a JavaScript animation.

## Generate a native trace

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/oblivion_3d_trace
```

On Windows, run `build/Release/oblivion_3d_trace.exe`. An optional first argument selects the output JSON path.

## Open the viewer

Serve the repository root using `python -m http.server 8000`, then open `http://localhost:8000/simulator/web/`. Select **Load trace** and choose `oblivion_3d_trace.json`.

The viewer provides orbit/zoom/pan, playback, playback speed, track scenarios, grid, wireframe, sensor-bar visibility and trajectory trail.

Three.js is loaded from jsDelivr, so the initial viewer load requires internet access.

## Coordinates

The native model uses right-handed Z-up coordinates (X forward, Y lateral, Z vertical). The Three.js viewer uses Y-up and converts the native pose and quaternion for display.

## Model limitations

This is an initial 3D physics and trace-visualization foundation, not a validated digital twin. The current plant uses a simplified four-corner suspension/contact approximation and simplified tire forces. Detailed suspension kinematics, calibrated tire relaxation, wheel steering geometry, motor electrical circuits, collision meshes, deformable terrain, sensor raycasting and a closed-loop Sprint controller running in the 3D plant remain future work. The generated trace currently exercises the native 3D plant with deterministic differential motor commands, not the full OBLIVION line-following stack.

All physical parameters are provisional until calibrated against a real robot.


# Standalone virtual robot endpoint

The simulator is a separate process, not an embedded Control Center mode. It exposes a virtual robot over the same OBLIVION framed TCP stream so the native desktop can consume telemetry and send mission commands as though it were connected to a robot.

## Run the protocol simulator

Requirements: Python 3.10+; standard library only.

```bash
python simulator/oblivion_simulator.py --host 127.0.0.1 --port 8765
```

Keep this process running, then choose **Simulator TCP** in Control Center and connect to host `127.0.0.1`, port `8765`. The simulator publishes typed telemetry v1 at approximately 40 Hz. Start/Pause/Stop and emergency-stop frames change the virtual robot's reported state. It never accesses physical hardware.

This is a protocol integration stub with synthetic analytical motion and sensor values, not yet the full 3D digital-twin simulator. It is separate so the desktop does not need to know whether the endpoint is virtual or physical. Its CPU, memory, battery and controller timing values are illustrative only, not performance measurements.
