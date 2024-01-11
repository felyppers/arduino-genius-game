// Test Code - Used to check if all components are working
// It will blink all leds set in the pins array first
// Then it will write in all possible spaces of the LCD 16x2 screen
// Finally it will enable the buttons so the user can press then and have a led blink feedback
// It is useful to check if all wires are correctly connected in your board

#include <LiquidCrystal.h>

bool start = true;
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
byte pins[] = { 2, 3, 4 };

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("A-B-C-D-E-F-G-H-");
  lcd.setCursor(0, 1);
  lcd.print("0-1-2-3-4-5-6-7-");
}

void setPinDirection(byte dir) {
  for (byte i = 0; i < sizeof(pins); i++) {
    pinMode(pins[i], dir);
  }
}

// Flashes all the LEDs together. "freq" is the blink speed, the higher the slower
void flash(short freq) {
  setPinDirection(OUTPUT); // Activate the LEDS
  for (int i = 0; i < 5; i++) {
    for (byte i = 0; i < sizeof(pins); i++) {
        digitalWrite(pins[i], HIGH);
    }
    delay(freq);
    for (byte i = 0; i < sizeof(pins); i++) {
        digitalWrite(pins[i], LOW);
    }
    delay(freq);
  }
}

void loop() {
    if (start) {
      start = false;
      flash(500);
    }
    setPinDirection(INPUT); // Activate the Buttons
}