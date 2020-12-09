#include "globals.h"
#include "messagebuffer.h"


//-----------------------------------------------------------------------------------------------//
// Globals section
//-----------------------------------------------------------------------------------------------//

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Output buffer for line one of the display
char g_messageBuffer1[DISPLAY_LINE_LENGTH + 1];
char g_messageBuffer2[DISPLAY_LINE_LENGTH + 1];

// Data ring buffer
uint8_t g_dataBuffer[DATA_BUFFER_SIZE];
MessageBuffer g_messageBuffer(g_dataBuffer, DATA_BUFFER_SIZE);

// Mode state
pfnMode g_currentMode;


//-----------------------------------------------------------------------------------------------//
// Modes
//-----------------------------------------------------------------------------------------------//

#define MODE(m) void m();
#define setMode(m) g_currentMode = m;

MODE(commandMode);
MODE(statusMode);

void commandMode() {
    display(1, "Idle");

    g_messageBuffer.reset();
    g_messageBuffer.readFromPort(1);
    switch (g_messageBuffer.get(0))
    {
        case 0x21: // Status message
            setMode(statusMode);
            break;

        default:
            halt(ERROR_UNEXPECTED_MESSAGE, g_messageBuffer.get(0));
    }
}

void statusMode() {
    uint8_t checksum;
    g_messageBuffer.readFromPort(2);
    g_messageBuffer.ensureValidMessage();
    switch (g_messageBuffer.get(1))
    {
        case 0x24: // Ping
            checksum = writePacket(0x62);
            checksum = writePacket(0x22, checksum);
            checksum = writePacket(0x40, checksum);
            checksum = writePacket(checksum);
            break;

        case 0x21: // Version info request
            checksum = writePacket(0x63);
            checksum = writePacket(0x21, checksum);
            checksum = writePacket(VERSION_HEX, checksum);
            checksum = writePacket(0x01, checksum);
            checksum = writePacket(checksum);
            break;

        default:
            halt(ERROR_UNEXPECTED_REQUEST, g_messageBuffer.get(1));
    }

    setMode(commandMode);
}


//-----------------------------------------------------------------------------------------------//
// Setup and main loop
//-----------------------------------------------------------------------------------------------//

void setup() {
    memset(g_messageBuffer1, 0, DISPLAY_LINE_LENGTH + 1);
    memset(g_messageBuffer2, 0, DISPLAY_LINE_LENGTH + 1);
    memset(g_dataBuffer, 0, DATA_BUFFER_SIZE);
    g_currentMode = commandMode;

    lcd.begin(16, 2);
    display(0, "aLink " VERSION);

    Serial.begin(PORT_SPEED);
}

void loop() {
    g_currentMode();
}