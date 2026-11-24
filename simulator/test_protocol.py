import unittest
from oblivion_simulator import Robot, frame, unpack_frame, TELEMETRY
from oblivion_simulator import MAX_PAYLOAD

class SimulatorProtocolTests(unittest.TestCase):
    def test_frame_crc_round_trip(self):
        payload = bytes(range(32))
        packet = unpack_frame(frame(TELEMETRY, 1234, payload))
        self.assertEqual(packet, (TELEMETRY, 1234, payload))

    def test_telemetry_v1_payload_size(self):
        robot = Robot()
        payload = robot.telemetry()
        self.assertEqual(len(payload), 134)
        self.assertLessEqual(len(payload), MAX_PAYLOAD)

    def test_start_stop_commands(self):
        robot = Robot()
        robot.command(2, bytes((1, 0, 0x3f, 0x02)))
        self.assertTrue(robot.active)
        robot.command(2, bytes((0, 0, 0x3f, 0x02)))
        self.assertFalse(robot.active)
        self.assertEqual(robot.speed, 0.0)

    def test_emergency_stop(self):
        robot = Robot()
        robot.active = True
        robot.speed = 0.5
        robot.command(4, b"")
        self.assertFalse(robot.active)
        self.assertEqual(robot.speed, 0.0)

if __name__ == "__main__":
    unittest.main()
