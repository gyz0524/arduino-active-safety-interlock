#include <LiquidCrystal.h>

// LCD1602 parallel wiring: RS, E, D4, D5, D6, D7.
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

const uint8_t TOUCH_PIN = 8;       // TTP223B OUT
const uint8_t JOY_BUTTON_PIN = 9;  // Joystick SW, active LOW
const uint8_t TILT_PIN = 10;       // SW-520D digital output
const uint8_t MATRIX_DIN = 11;     // MAX7219-style matrix DIN
const uint8_t MATRIX_CS = 12;      // MAX7219-style matrix CS
const uint8_t MATRIX_CLK = 13;     // MAX7219-style matrix CLK
const uint8_t JOY_X_PIN = A0;      // Joystick VRx
const uint8_t BUZZER_PIN = A1;     // Buzzer module S/IN

// Most SW-520D modules pull LOW when tilted. Change to HIGH if needed.
const uint8_t TILT_ACTIVE_LEVEL = LOW;

const unsigned long STEP_TIMEOUT_MS = 8000UL;
const unsigned long TILT_HOLD_MS = 350UL;
const int JOY_LEFT_THRESHOLD = 350;
const int JOY_RIGHT_THRESHOLD = 670;

enum TaskState {
  IDLE,
  WAIT_JOYSTICK,
  WAIT_TILT,
  SUCCESS,
  FAIL
};

enum Direction {
  CENTER,
  LEFT,
  RIGHT
};

TaskState state = IDLE;
const Direction targetDirection = LEFT;
unsigned long stepDeadline = 0;
unsigned long tiltStartedAt = 0;
int lastDisplayedSeconds = -1;

const byte CHECK_ICON[8] = {
  B00000000,
  B00000001,
  B00000011,
  B00000110,
  B10001100,
  B11011000,
  B01110000,
  B00100000
};

const byte CROSS_ICON[8] = {
  B10000001,
  B01000010,
  B00100100,
  B00011000,
  B00011000,
  B00100100,
  B01000010,
  B10000001
};

void max7219Write(byte address, byte value) {
  digitalWrite(MATRIX_CS, LOW);
  shiftOut(MATRIX_DIN, MATRIX_CLK, MSBFIRST, address);
  shiftOut(MATRIX_DIN, MATRIX_CLK, MSBFIRST, value);
  digitalWrite(MATRIX_CS, HIGH);
}

void clearMatrix() {
  for (byte row = 1; row <= 8; row++) {
    max7219Write(row, 0x00);
  }
}

void showMatrixIcon(const byte icon[8]) {
  for (byte row = 0; row < 8; row++) {
    max7219Write(row + 1, icon[row]);
  }
}

void initMatrix() {
  pinMode(MATRIX_DIN, OUTPUT);
  pinMode(MATRIX_CS, OUTPUT);
  pinMode(MATRIX_CLK, OUTPUT);
  digitalWrite(MATRIX_CS, HIGH);

  max7219Write(0x09, 0x00);  // no decode
  max7219Write(0x0A, 0x04);  // medium brightness
  max7219Write(0x0B, 0x07);  // scan all eight rows
  max7219Write(0x0C, 0x01);  // normal operation
  max7219Write(0x0F, 0x00);  // display test off
  clearMatrix();
}

void playStepCorrectSound() {
  tone(BUZZER_PIN, 1200, 90);
  delay(115);
  tone(BUZZER_PIN, 1600, 130);
}

void playErrorSound() {
  tone(BUZZER_PIN, 300, 180);
  delay(220);
  tone(BUZZER_PIN, 220, 260);
}

void playFinalCorrectSound() {
  tone(BUZZER_PIN, 900, 100);
  delay(120);
  tone(BUZZER_PIN, 1200, 100);
  delay(120);
  tone(BUZZER_PIN, 1600, 220);
  delay(240);
  noTone(BUZZER_PIN);
}

void showIdle() {
  lcd.clear();
  lcd.print(F("TOUCH TO START"));
  lcd.setCursor(0, 1);
  lcd.print(F("3 STEP TASK"));
  clearMatrix();
}

void showJoystickStep() {
  lcd.clear();
  lcd.print(F("STEP1 LEFT T:"));
  lcd.setCursor(0, 1);
  lcd.print(F("PRESS JOY + BTN"));
  lastDisplayedSeconds = -1;
}

void showTiltStep() {
  lcd.clear();
  lcd.print(F("STEP2 TILT T:"));
  lcd.setCursor(0, 1);
  lcd.print(F("HOLD 0.4 SEC"));
  lastDisplayedSeconds = -1;
}

void showSuccess() {
  lcd.clear();
  lcd.print(F("ALL STEPS OK"));
  lcd.setCursor(0, 1);
  lcd.print(F("CHECK MARK"));
  showMatrixIcon(CHECK_ICON);
  playFinalCorrectSound();
}

void showFailure() {
  lcd.clear();
  lcd.print(F("WRONG - STOP"));
  lcd.setCursor(0, 1);
  lcd.print(F("TOUCH TO RESET"));
  showMatrixIcon(CROSS_ICON);
  playErrorSound();
}

bool touchPressed() {
  static bool previous = false;
  const bool current = digitalRead(TOUCH_PIN) == HIGH;
  const bool pressed = current && !previous;
  previous = current;
  return pressed;
}

bool joystickButtonPressed() {
  static bool lastReading = HIGH;
  static bool stableState = HIGH;
  static unsigned long changedAt = 0;
  const bool reading = digitalRead(JOY_BUTTON_PIN);
  const unsigned long now = millis();

  if (reading != lastReading) {
    lastReading = reading;
    changedAt = now;
  }

  if ((now - changedAt) >= 25UL && reading != stableState) {
    stableState = reading;
    return stableState == LOW;
  }
  return false;
}

Direction readJoystickDirection() {
  const int x = analogRead(JOY_X_PIN);
  if (x < JOY_LEFT_THRESHOLD) {
    return LEFT;
  }
  if (x > JOY_RIGHT_THRESHOLD) {
    return RIGHT;
  }
  return CENTER;
}

void startTask() {
  state = WAIT_JOYSTICK;
  stepDeadline = millis() + STEP_TIMEOUT_MS;
  tiltStartedAt = 0;
  showJoystickStep();
}

void failTask() {
  state = FAIL;
  stepDeadline = 0;
  tiltStartedAt = 0;
  showFailure();
}

void completeTask() {
  state = SUCCESS;
  stepDeadline = 0;
  tiltStartedAt = 0;
  showSuccess();
}

void updateCountdown(unsigned long now) {
  if (state != WAIT_JOYSTICK && state != WAIT_TILT) {
    return;
  }

  const int secondsLeft = (stepDeadline > now)
                            ? (int)((stepDeadline - now + 999UL) / 1000UL)
                            : 0;
  if (secondsLeft == lastDisplayedSeconds) {
    return;
  }

  lastDisplayedSeconds = secondsLeft;
  lcd.setCursor(13, 0);
  lcd.print(F("   "));
  lcd.setCursor(13, 0);
  lcd.print(secondsLeft);
}

void setup() {
  pinMode(TOUCH_PIN, INPUT);
  pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(TILT_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  delay(50);
  lcd.begin(16, 2);
  initMatrix();
  showIdle();
}

void loop() {
  const unsigned long now = millis();
  updateCountdown(now);

  if (state == IDLE) {
    if (touchPressed()) {
      startTask();
    }
    return;
  }

  if (state == WAIT_JOYSTICK) {
    if (now >= stepDeadline) {
      failTask();
      return;
    }

    if (joystickButtonPressed()) {
      if (readJoystickDirection() == targetDirection) {
        playStepCorrectSound();
        state = WAIT_TILT;
        stepDeadline = millis() + STEP_TIMEOUT_MS;
        tiltStartedAt = 0;
        showTiltStep();
      } else {
        failTask();
      }
    }
    return;
  }

  if (state == WAIT_TILT) {
    if (now >= stepDeadline) {
      failTask();
      return;
    }

    if (digitalRead(TILT_PIN) == TILT_ACTIVE_LEVEL) {
      if (tiltStartedAt == 0) {
        tiltStartedAt = now;
      }
      if ((now - tiltStartedAt) >= TILT_HOLD_MS) {
        completeTask();
      }
    } else {
      tiltStartedAt = 0;
    }
    return;
  }

  if ((state == SUCCESS || state == FAIL) && touchPressed()) {
    state = IDLE;
    noTone(BUZZER_PIN);
    showIdle();
  }
}
