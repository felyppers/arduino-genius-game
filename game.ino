#include <LiquidCrystal.h>

#define TURN_DURATION 2000                // The time allowed between button presses in miliseconds

byte sequence[100];                       // Storage for the light sequence
int score = 0;                            // Current length of the sequence
byte inputCount = 0;                      // The number of times that the player has pressed a correct button in a given turn
byte lastInput = 0;                       // Last input from the player
byte expectedAnswer = 0;                  // The LED that's suppose to be lit by the player
bool btnDwn = false;                      // Flag to check if a button is pressed
bool playersTurn = false;                 // Flag to check if the program is waiting for the user to press a button
bool printPlayerMessage = true;           // Flag to check if the program must print a message during player's turn
bool resetFlag = false;                   // Flag used to indicate to the program that the player has lost
long inputTime = 0;                       // Timer for the delay between user inputs
String lcdMessage;                        // String to set concatenated lcd messages

// Custom variables: These need to be set according to your board setup
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);  // Setup LCD monitor with the needed pins
byte pins[] = { 2, 3, 4 };                // Button input pins and LED output pins

// Setup the initial game state
void setup() {
  lcd.begin(16, 2);                       // Start LCD monitor
  setLcdEvent("SETUP");                   // Set SETUP LCD message
  Serial.begin(9600);                     // Start Serial monitor
  delay(3000);
  reset();
}

// Sets all the pins as either INPUT (for LEDs) or OUTPUT (for Buttons)
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

// This function resets all the game variables to their default values
void reset() {
  setLcdEvent("NEWGAME");
  delay(1000);
  flash(500);
  score = 0;
  inputCount = 0;
  lastInput = 0;
  expectedAnswer = 0;
  btnDwn = false;
  playersTurn = false;
  resetFlag = false;
}

// The arduino shows the user the current sequence (needed to replay the sequence every turn and also after game over)
void playSequence() {
  // Loop through the stored sequence and light the appropriate LEDs in turn
  for (int i = 0; i < score; i++) {
    digitalWrite(sequence[i], HIGH);
    delay(500);
    digitalWrite(sequence[i], LOW);
    delay(250);
  }
}

// Clears the LCD monitor
void clearLcd() {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

// Set custom messages to LCD monitor based on an event
void setLcdEvent(String event) {
  clearLcd();
  lcd.setCursor(0, 0);
  if (event == "SETUP") {
    lcd.print("     GENIUS     ");
    lcd.setCursor(0, 1);
    lcd.print("  memory  game  ");
  } else if (event == "NEWGAME") {
    lcd.print("    NEW GAME    ");
    lcd.setCursor(0, 1);
    lcd.print("learn and repeat");
  } else {
    if (event == "GAMEOVER") {
      lcdMessage = "Final Score: " + String(score - 1);
      lcd.print("   GAME  OVER   ");
    } else if (event == "CORRECT") {
      lcdMessage = "Score: " + String(score);
      lcd.print("   WELL DONE!   ");
    } else {
      lcdMessage = "Score: " + String(score - 1);
      if (event == "LEARN") {
        lcd.print("SEE THE SEQUENCE");
      } else if (event == "PLAY") {
        lcd.print(" NOW REPEAT IT! ");
      }
    }
    lcd.setCursor(0, 1);
    lcd.print(lcdMessage);
  }
}

// The events that occur upon a loss
void gameOver() {
  setLcdEvent("GAMEOVER");
  flash(50);                                            // Flash all the LEDS quickly
  flash(50);
  flash(50);
  delay(1000);
  playSequence();                                       // Shows the user the last sequence
  delay(1000);
  reset();                                              // Reset everything for a new game
}

// Arduino main loop
void loop() {
  if (!playersTurn) {
    // Arduino's turn
    setPinDirection(OUTPUT);                            // Using the LEDs
    randomSeed(analogRead(A0));                         // https://www.arduino.cc/en/Reference/RandomSeed
    sequence[score] = pins[random(0, sizeof(pins))];    // Put a new random value in the next position in the sequence - https://www.arduino.cc/en/Reference/random

    if (score > 0) {
      setLcdEvent("CORRECT");                           // Informs a correct answer from the user in the LCD monitor
      delay(1000);
    }

    printPlayerMessage = true;                          // Updates flag used to update LCD message with PLAYER'S TURN event
    score++;                                            // Set the new score (which is used as also the length of the sequence)

    setLcdEvent("LEARN");                              // Informs the user it is time to memorize the sequence in the LCD monitor
    delay(1000);
    playSequence();                                     // Show the sequence to the player

    playersTurn = true;                                 // Set playersTurn flag to true as it's now going to be the turn of the player
    inputTime = millis();                               // Store the time to measure the player's response time
  } else {
    // Player's turn
    if (printPlayerMessage) {
      setLcdEvent("PLAY");                              // Informs the user it is time to play the memorized sequence in the LCD monitor
      printPlayerMessage = false;                       // Updates flag used to update LCD message with PLAYER'S TURN event (otherwise it will keep updating the LCD every cycle)
    }

    setPinDirection(INPUT);                             // We're using the buttons
    if (millis() - inputTime > TURN_DURATION) {         // If the player takes more than the allowed time, lose
      gameOver();
      return;
    }
    if (!btnDwn) {
      expectedAnswer = sequence[inputCount];            // Find the value we expect from the player
      for (int i = 0; i < sizeof(pins); i++) {          // Loop through the all the pins
        if (pins[i] == expectedAnswer)
          continue;                                     // Ignore the correct pin
        if (digitalRead(pins[i]) == HIGH) {             // Is the button pressed
          lastInput = pins[i];
          resetFlag = true;                             // Set the resetFlag - this means the player has lost
          btnDwn = true;                                // This will prevent the program from doing the same thing over and over again
        }
      }
    }

    if (digitalRead(expectedAnswer) == 1 && !btnDwn) {  // The player pressed the right button
      inputTime = millis();
      lastInput = expectedAnswer;
      inputCount++;                                     // The user pressed a (correct) button again
      btnDwn = true;                                    // This will prevent the program from doing the same thing over and over again
    } else {
      if (btnDwn && digitalRead(lastInput) == LOW) {    // Check if the player released the button
        btnDwn = false;
        delay(20);
        if (resetFlag) {                                // If this was set to true up above, you lost
          gameOver();                                   // So we do the losing sequence of events
        } else {
          if (inputCount == score) {                    // Has the player finished repeating the sequence
            playersTurn = false;                        // If so, this will make the next turn the program's turn
            inputCount = 0;                             // Reset the number of times that the player has pressed a button
            delay(1500);
          }
        }
      }
    }
  }
}
