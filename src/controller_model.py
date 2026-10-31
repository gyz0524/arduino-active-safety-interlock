from enum import Enum, auto


class ControllerState(Enum):
    IDLE = auto()
    WAIT_JOYSTICK = auto()
    WAIT_TILT = auto()
    SUCCESS = auto()
    FAIL = auto()


class TaskController:
    def __init__(self, step_timeout_ms=3000):
        self.step_timeout_ms = step_timeout_ms
        self.state = ControllerState.IDLE
        self.target_direction = None
        self.deadline_ms = None
        self.outcome = None

    def start(self, now_ms):
        self.state = ControllerState.WAIT_JOYSTICK
        self.target_direction = "LEFT"
        self.deadline_ms = now_ms + self.step_timeout_ms
        self.outcome = None

    def joystick(self, direction, now_ms):
        if self.state != ControllerState.WAIT_JOYSTICK:
            return
        if now_ms >= self.deadline_ms or direction != self.target_direction:
            self.state = ControllerState.FAIL
            self.deadline_ms = None
            self.outcome = "CROSS"
            return
        self.state = ControllerState.WAIT_TILT
        self.deadline_ms = now_ms + self.step_timeout_ms

    def tilt(self, now_ms):
        if self.state != ControllerState.WAIT_TILT:
            return
        if now_ms >= self.deadline_ms:
            self.state = ControllerState.FAIL
            self.deadline_ms = None
            self.outcome = "CROSS"
            return
        self.state = ControllerState.SUCCESS
        self.deadline_ms = None
        self.outcome = "CHECK"

    def tick(self, now_ms):
        if self.state in (ControllerState.WAIT_JOYSTICK, ControllerState.WAIT_TILT):
            if now_ms >= self.deadline_ms:
                self.state = ControllerState.FAIL
                self.deadline_ms = None
                self.outcome = "CROSS"

    def reset(self):
        self.state = ControllerState.IDLE
        self.target_direction = None
        self.deadline_ms = None
        self.outcome = None
