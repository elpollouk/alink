#include <LiquidCrystal.h>

#define VERSION "1.07"
#define VERSION_HEX 0x6B
#define MESSAGE_BUFFER_SIZE 16
#define DATA_BUFFER_SIZE 16
#define PORT_SPEED 115200

typedef uint8_t Handle;
typedef void (*pfnMode)(void);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Output buffer for line one of the display
char g_messageBuffer1[MESSAGE_BUFFER_SIZE + 1];
char g_messageBuffer2[MESSAGE_BUFFER_SIZE + 1];
// Data ring buffer
uint8_t g_dataBuffer[DATA_BUFFER_SIZE];
uint8_t g_dataIndex = 0;
// Mode state
pfnMode g_currentMode;

void display(uint8_t line, const char* message) {
    lcd.setCursor(0, line);
    lcd.print(message);
}

void halt(const char* message) {
    display(0, "ERROR           ");
    lcd.setCursor(0, 1);

    uint8_t i;
    for (i = 0; i < 16; i++) {
        if (message[i] == 0) break;
        lcd.print(message[i]);
    }

    for(; i < 16; i++) {
        lcd.print(' ');
    }

    while (true) {};
}

char nibbleToHex(uint8_t nibble) {
    if (nibble < 10) {
        return '0' + nibble;
    }
    return 'A' + (nibble - 10);
}

void render(char * target, uint8_t fromIndex) {
    // Render a single line of bytes (8 bytes -> 16 hex chars)
    for (uint8_t i = 0; i < DATA_BUFFER_SIZE / 2; i++) {
        uint8_t byte = g_dataBuffer[(g_dataIndex + fromIndex + i) % DATA_BUFFER_SIZE];
        target[i * 2] = nibbleToHex(byte >> 4);
        target[(i * 2) + 1] = nibbleToHex(byte & 0xF);
    }
}

void appendDataBuffer(uint8_t data) {
    // Update the ring buffer
    g_dataBuffer[g_dataIndex++ % DATA_BUFFER_SIZE] = data;

    // Render the message to text lines
    render(g_messageBuffer1, 0);
    render(g_messageBuffer2, DATA_BUFFER_SIZE / 2);

    // Display message
    display(0, g_messageBuffer1);
    display(1, g_messageBuffer2);
}

Handle readIntoBuffer(uint8_t count) {
    Handle startIndex = g_dataIndex;
    while (count != 0) {
        // Receive data into buffer
        if (Serial.available()) {
            uint8_t inChar = (uint8_t)Serial.read();
            appendDataBuffer(inChar);
            count--;
        }
    }
    return startIndex;
}

uint8_t readByteFromBuffer(Handle& index) {
    return g_dataBuffer[index++ % DATA_BUFFER_SIZE];
}

uint8_t write(uint8_t byte, uint8_t checkSum) {
    Serial.write(byte);
    return checkSum ^ byte;
}

bool isValidMessage(Handle h, uint8_t size) {
    uint8_t v = 0;
    while (size--)
        v ^= readByteFromBuffer(h);
    
    return v == 0;
}

void commandMode() {
    uint8_t checksum;
    Handle h = readIntoBuffer(3);
    if (!isValidMessage(h, 3)) halt("Checksum error");
    if (readByteFromBuffer(h) != 0x21) halt("Unxpected msg");
    switch (readByteFromBuffer(h))
    {
        case 0x24: // Ping
            checksum = write(0x62, 0);
            checksum = write(0x22, checksum);
            checksum = write(0x40, checksum);
            checksum = write(checksum, 0);
            break;

        case 0x21: // Version info request
            checksum = write(0x63, 0);
            checksum = write(0x21, checksum);
            checksum = write(VERSION_HEX, checksum);
            checksum = write(0x01, checksum);
            checksum = write(checksum, 0);
            break;

        default:
            halt("Unexpected req");
    }
}

void setup() {
    memset(g_messageBuffer1, 0, MESSAGE_BUFFER_SIZE + 1);
    memset(g_messageBuffer2, 0, MESSAGE_BUFFER_SIZE + 1);
    memset(g_dataBuffer, 0, DATA_BUFFER_SIZE);
    g_currentMode = commandMode;

    lcd.begin(16, 2);
    display(0, "aLink " VERSION);
    display(1, "Waiting...");

    Serial.begin(PORT_SPEED);
}

void loop() {
    g_currentMode();
}