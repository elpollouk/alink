#include <LiquidCrystal.h>

#define BUFFER_SIZE 16

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

char message_buffer[BUFFER_SIZE + 1];
int message_index = 0;

void setup() {
  memset(message_buffer, 0, BUFFER_SIZE + 1);
  lcd.begin(16, 2);
  lcd.print("aLink 0.1 - ");

  Serial.begin(9600);
}

void loop() {
  // Receive data into buffer
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    switch (inChar) {
      case '\n':
        message_index = 0;
        break;

      default:
        if (message_index >= BUFFER_SIZE) break;
        message_buffer[message_index++] = inChar;
        message_buffer[message_index] = 0;
        break;
    }
  }

  // Display time
  lcd.setCursor(12, 0);
  lcd.print(millis() / 1000);

  // Display message
  lcd.setCursor(0, 1);
  lcd.print(message_buffer);
}