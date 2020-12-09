#include "globals.h"
#include "messagebuffer.h"

void MessageBuffer::append(uint8_t byte) {
    m_buffer[m_writeIndex++] = byte;
}

void MessageBuffer::reset() {
    m_writeIndex = 0;
}

void MessageBuffer::readFromPort(uint8_t numBytes) {
    if (m_writeIndex + numBytes > m_size) halt(ERROR_BUFFER_FULL, m_writeIndex + numBytes);

    while (numBytes != 0) {
        // Receive data into buffer
        if (Serial.available()) {
            uint8_t inChar = (uint8_t)Serial.read();
            append(inChar);
            numBytes--;
        }
    }
}

void MessageBuffer::ensureValidMessage() const  {
    uint8_t v = 0;
    for (uint8_t i = 0; i < m_writeIndex; i++)
        v ^= m_buffer[i];
    
    if (v != 0) halt(ERROR_INVALID_CHECKSUM, v);
}