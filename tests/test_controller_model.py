import unittest

from src.controller_model import ControllerState, TaskController


class TaskControllerTests(unittest.TestCase):
    def test_start_waits_for_the_requested_joystick_direction(self):
        controller = TaskController(step_timeout_ms=3000)

        controller.start(now_ms=1000)

        self.assertEqual(controller.state, ControllerState.WAIT_JOYSTICK)
        self.assertEqual(controller.target_direction, "LEFT")
        self.assertEqual(controller.deadline_ms, 4000)

    def test_correct_joystick_then_tilt_allows_success(self):
        controller = TaskController(step_timeout_ms=3000)
        controller.start(now_ms=1000)

        controller.joystick("LEFT", now_ms=1500)
        self.assertEqual(controller.state, ControllerState.WAIT_TILT)

        controller.tilt(now_ms=2000)
        self.assertEqual(controller.state, ControllerState.SUCCESS)
        self.assertEqual(controller.outcome, "CHECK")

    def test_wrong_input_or_timeout_fails_and_requires_reset(self):
        controller = TaskController(step_timeout_ms=3000)
        controller.start(now_ms=1000)
        controller.joystick("RIGHT", now_ms=1500)
        self.assertEqual(controller.state, ControllerState.FAIL)
        self.assertEqual(controller.outcome, "CROSS")

        controller.reset()
        controller.start(now_ms=5000)
        controller.tick(now_ms=8000)
        self.assertEqual(controller.state, ControllerState.FAIL)
        self.assertEqual(controller.outcome, "CROSS")


if __name__ == "__main__":
    unittest.main()
