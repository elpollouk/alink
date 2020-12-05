#include <LiquidCrystal.h>

#define VERSION "0.1"
#define MESSAGE_BUFFER_SIZE 16
#define DATA_BUFFER_SIZE 8
#define PORT_SPEED 115200

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

char message_buffer[MESSAGE_BUFFER_SIZE + 1];
uint8_t data_buffer[DATA_BUFFER_SIZE];
int message_index = 0;
int data_index = 0;
bool clear_message = false;
bool update_message = false;


void appendMessageBuffer(char data) {
  if (message_index >= MESSAGE_BUFFER_SIZE) return;
  message_buffer[message_index++] = data;
  message_buffer[message_index] = 0;
  update_message = true;
}

char nibbleToHex(uint8_t nibble) {
  if (nibble < 10) {
    return '0' + nibble;
  }
  return 'A' + (nibble - 10);
}

void appendDataBuffer(char data) {
  data_buffer[data_index++ % DATA_BUFFER_SIZE] = data;
  
  message_index = 0;
  for (int i = 0; i < DATA_BUFFER_SIZE; i++) {
    uint8_t byte = data_buffer[(data_index + i) % DATA_BUFFER_SIZE];
    appendMessageBuffer(nibbleToHex(byte >> 4));
    appendMessageBuffer(nibbleToHex(byte & 0xF));
  }
}

void setup() {
  memset(message_buffer, 0, MESSAGE_BUFFER_SIZE + 1);
  memset(data_buffer, 0, DATA_BUFFER_SIZE);
  lcd.begin(16, 2);
  lcd.print("aLink " VERSION " - ");

  Serial.begin(PORT_SPEED);
}

void loop() {
  // Receive data into buffer
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    /*if  (inChar == '\n') {
      message_index = 0;
      clear_message = true;
    }
    else {
      appendMessageBuffer(inChar);
    }*/
    appendDataBuffer(inChar);
  }

  // Display time
  lcd.setCursor(12, 0);
  lcd.print(millis() / 1000);

  if (update_message) {
    if (clear_message) {
      // Start of a new message, so clear out the previous message,
      lcd.setCursor(0, 1);
      lcd.print("                ");
      clear_message = false;
    }

    // Display message
    lcd.setCursor(0, 1);
    lcd.print(message_buffer);
    update_message = false;
  }
}