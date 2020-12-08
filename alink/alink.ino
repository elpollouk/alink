#include <LiquidCrystal.h>

#define VERSION "0.1"
#define MESSAGE_BUFFER_SIZE 16
#define DATA_BUFFER_SIZE 16
#define PORT_SPEED 115200

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Output buffer for line one of the display
char message_buffer_1[MESSAGE_BUFFER_SIZE + 1];
char message_buffer_2[MESSAGE_BUFFER_SIZE + 1];
// Data ring buffer
uint8_t data_buffer[DATA_BUFFER_SIZE];
uint8_t data_index = 0;

char nibbleToHex(uint8_t nibble) {
    if (nibble < 10) {
        return '0' + nibble;
    }
    return 'A' + (nibble - 10);
}

void render(char * target, uint8_t fromIndex) {
    // Render a single line of bytes (8 bytes -> 16 hex chars)
    for (int i = 0; i < DATA_BUFFER_SIZE / 2; i++) {
        uint8_t byte = data_buffer[(data_index + fromIndex + i) % DATA_BUFFER_SIZE];
        target[i * 2] = nibbleToHex(byte >> 4);
        target[(i * 2) + 1] = nibbleToHex(byte & 0xF);
    }
}

void appendDataBuffer(uint8_t data) {
    // Update the ring buffer
    data_buffer[data_index++ % DATA_BUFFER_SIZE] = data;

    // Render the message to text lines
    render(message_buffer_1, 0);
    render(message_buffer_2, DATA_BUFFER_SIZE / 2);

    // Display message
    lcd.setCursor(0, 0);
    lcd.print(message_buffer_1);
    lcd.setCursor(0, 1);
    lcd.print(message_buffer_2);
}

void setup() {
    memset(message_buffer_1, 0, MESSAGE_BUFFER_SIZE + 1);
    memset(message_buffer_2, 0, MESSAGE_BUFFER_SIZE + 1);
    memset(data_buffer, 0, DATA_BUFFER_SIZE);
    lcd.begin(16, 2);
    lcd.print("aLink " VERSION);
    lcd.setCursor(0, 1);
    lcd.print("Waiting...");

    Serial.begin(PORT_SPEED);
}

void loop() {
    // Receive data into buffer
    while (Serial.available()) {
        uint8_t inChar = (uint8_t)Serial.read();
        appendDataBuffer(inChar);
    }
}