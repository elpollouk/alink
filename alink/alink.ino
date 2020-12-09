#include "globals.h"
#include "messagebuffer.h"


//-----------------------------------------------------------------------------------------------//
// Globals section
//-----------------------------------------------------------------------------------------------//

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Data ring buffer
uint8_t g_dataBuffer[DATA_BUFFER_SIZE];
MessageBuffer g_messageBuffer(g_dataBuffer, sizeof(g_dataBuffer));

// Mode state
pfnMode g_currentMode;


//-----------------------------------------------------------------------------------------------//
// Modes
//-----------------------------------------------------------------------------------------------//

#define DEF_MODE(m) void mode_ ## m();
#define MODE(m) void mode_ ## m()
#define setMode(m) g_currentMode = mode_ ## m;

DEF_MODE(command);
DEF_MODE(status);
DEF_MODE(locoSpeed);

MODE(command) {
    debugMessage("Idle");

    g_messageBuffer.reset();
    g_messageBuffer.readFromPort(1);
    switch (g_messageBuffer.get(0))
    {
        case 0x21: // Status request
            setMode(status);
            break;

        case 0xE4: // Set loco speed
            setMode(locoSpeed);
            break;

        default:
            halt(ERROR_UNEXPECTED_MESSAGE, g_messageBuffer.get(0));
    }
}

MODE(status) {
    debugMessage("Status?");

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

    debugDelay();
    setMode(command);
}

MODE(locoSpeed) {
    debugMessage("Speed");
    g_messageBuffer.readFromPort(5);
    g_messageBuffer.ensureValidMessage();

    debugDelay();
    setMode(command);
}

//-----------------------------------------------------------------------------------------------//
// Setup and main loop
//-----------------------------------------------------------------------------------------------//

void setup() {
    lcd.begin(16, 2);
    display(0, "aLink " VERSION);

    Serial.begin(PORT_SPEED);

    setMode(command);
}

void loop() {
    g_currentMode();
}