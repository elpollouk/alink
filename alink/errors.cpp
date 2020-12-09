#include  "globals.h"

void halt(const char* message, uint8_t code) {
    display(0, "ERROR");

    lcd.setCursor(0, 1);
    int count = lcd.print(message) + 3;
    lcd.write(' ');
    lcd.write(nibbleToHex(code >> 4));
    lcd.write(nibbleToHex(code & 0xF));

    for (; count < DISPLAY_LINE_LENGTH; count++) {
        lcd.write(' ');
    }

    while (true) {};
}