#include "globals.h"

void display(uint8_t line, const char* message) {
    lcd.setCursor(0, line);
    for (int count = lcd.print(message); count < DISPLAY_LINE_LENGTH; count++) {
        lcd.write(' ');
    }
}

char nibbleToHex(uint8_t nibble) {
    if (nibble < 10) {
        return '0' + nibble;
    }
    return 'A' + (nibble - 10);
}

uint8_t writeMessage(uint8_t byte, uint8_t checkSum) {
    // Update a rolling checksum as we send bytes
    Serial.write(byte);
    return checkSum ^ byte;
}
