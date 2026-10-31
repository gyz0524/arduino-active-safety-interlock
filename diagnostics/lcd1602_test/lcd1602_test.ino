#include <LiquidCrystal.h>

// This standalone test uses only the LCD. Keep the same wiring as 111.ino.
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

void setup() {
  delay(100);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD TEST OK");
  lcd.setCursor(0, 1);
  lcd.print("RS D2 E D3");
}

void loop() {
}
