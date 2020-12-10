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

// CV Values
uint8_t g_currentCv = 0;
uint8_t g_cvs[256];


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

        case 0x22: // CV select
            setMode(cvSelect);
            break;

        case 0x23: // CV write
            setMode(cvWrite);
            break;

        case 0x3A: // Handshake part 1
            setMode(handshake1);
            break;

        case 0x35: // Handshake part 2
            setMode(handshake2);
            break;

        case 0x52: // Mystery message
            setMode(mystery);
            break;

        case 0xE4: // Loco control
            setMode(locoControl);
            break;

        case 0xF0: // Ignore Arduino reset messages
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
        case 0x10: // CV read
            checksum = writeMessage(0x63);
            checksum = writeMessage(0x14, checksum);
            checksum = writeMessage(g_currentCv, checksum);
            checksum = writeMessage(g_cvs[g_currentCv], checksum);
            writeMessage(checksum);
            break;

        case 0x21: // Version info request
            checksum = writeMessage(0x63);
            checksum = writeMessage(0x21, checksum);
            checksum = writeMessage(VERSION_HEX, checksum);
            checksum = writeMessage(0x01, checksum);
            writeMessage(checksum);
            break;

        case 0x24: // Ping
            checksum = writeMessage(0x62);
            checksum = writeMessage(0x22, checksum);
            checksum = writeMessage(0x40, checksum);
            writeMessage(checksum);
            break;

        default:
            halt(ERROR_UNEXPECTED_REQUEST, g_messageBuffer[1]);
    }

    debugDelay();
    setMode(command);
}

MODE(handshake1) {
    debugMessage("Handshake 1");
    g_messageBuffer.recvMessage(12);

    auto checksum = writeMessage(0x35);
    checksum = writeMessage(0xA3, checksum);
    checksum = writeMessage(0x49, checksum);
    checksum = writeMessage(0xB3, checksum);
    checksum = writeMessage(0xA6, checksum);
    checksum = writeMessage(0xC1, checksum);
    writeMessage(checksum);

    debugDelay();
    setMode(command);
}

MODE(handshake2) {
    debugMessage("Handshake 2");
    g_messageBuffer.recvMessage(7);

    auto checksum = writeMessage(0x01);
    checksum = writeMessage(0x04);
    writeMessage(checksum);

    debugDelay();
    setMode(command);
}

MODE(locoControl) {
    debugMessage("Loco");
    g_messageBuffer.recvMessage(6);

    debugDelay();
    setMode(command);
}

void sendCvResponse() {
    uint8_t checksum;
    for (auto i = 0; i < 3; i++) {
        checksum = writeMessage(0x61);
        checksum = writeMessage(0x02, checksum);
        writeMessage(checksum);
    }
    for (auto i = 0; i < 3; i++) {
        checksum = writeMessage(0x61);
        checksum = writeMessage(0x01, checksum);
        writeMessage(checksum);
    }
}

MODE(cvSelect) {
    debugMessage("CV Select");
    g_messageBuffer.recvMessage(4);

    g_currentCv = g_messageBuffer[2];

    sendCvResponse();
    debugDelay();
    setMode(command);
}

MODE(cvWrite) {
    debugMessage("CV Write");
    g_messageBuffer.recvMessage(5);

    g_currentCv = g_messageBuffer[2];
    g_cvs[g_currentCv] = g_messageBuffer[3];

    sendCvResponse();
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

    memset(g_cvs, 0, sizeof(g_cvs));
    g_cvs[1] = 3;
    g_cvs[3] = 5;
    g_cvs[4] = 5;
    g_cvs[7] = CV_VERSION;
    g_cvs[8] = CV_MANUFACTURER;
    g_cvs[10] = 127;
    g_cvs[29] = 6;

    setMode(command);
}

void loop() {
    g_currentMode();
}