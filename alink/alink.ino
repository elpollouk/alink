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

#define MODE(m) void mode_ ## m()
#define setMode(m) g_currentMode = mode_ ## m;

MODE(command) {
    debugMessage("Idle");

    g_messageBuffer.reset();
    g_messageBuffer.readFromPort(1);
    switch (g_messageBuffer[0])
    {
        case 0x21: // Status request
            setMode(status);
            break;

        case 0xE4: // Set loco speed
            setMode(locoSpeed);
            break;

        case 0x52: // Mystery message
            setMode(mystery);
            break;

        default:
            halt(ERROR_UNEXPECTED_MESSAGE, g_messageBuffer[0]);
    }
}

MODE(status) {
    debugMessage("Status");

    uint8_t checksum;
    g_messageBuffer.recvMessage(3);
    switch (g_messageBuffer[1])
    {
        case 0x24: // Ping
            checksum = writeMessage(0x62);
            checksum = writeMessage(0x22, checksum);
            checksum = writeMessage(0x40, checksum);
            checksum = writeMessage(checksum);
            break;

        case 0x21: // Version info request
            checksum = writeMessage(0x63);
            checksum = writeMessage(0x21, checksum);
            checksum = writeMessage(VERSION_HEX, checksum);
            checksum = writeMessage(0x01, checksum);
            checksum = writeMessage(checksum);
            break;

        default:
            halt(ERROR_UNEXPECTED_REQUEST, g_messageBuffer[1]);
    }

    debugDelay();
    setMode(command);
}

MODE(locoSpeed) {
    debugMessage("Speed");
    g_messageBuffer.recvMessage(6);

    debugDelay();
    setMode(command);
}

MODE(mystery) {
    debugMessage("Mystery?");
    g_messageBuffer.recvMessage(4);

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