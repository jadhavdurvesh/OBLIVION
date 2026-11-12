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
