#pragma once

class MessageBuffer
{
private:
    uint8_t* m_buffer;
    uint8_t m_size;
    uint8_t m_writeIndex;

    void append(uint8_t byte);

public:
    MessageBuffer(uint8_t* buffer, uint8_t size):
        m_buffer(buffer),
        m_size(size),
        m_writeIndex(0)
    {

    }

    void reset();
    void readFromPort(uint8_t numBytes);
    void ensureValidMessage() const;

    const uint8_t& bytesAvailable() const {
        return m_writeIndex;
    }

    const uint8_t& get(uint8_t index) const {
        ASSERT(index < m_writeIndex, ERROR_BUFFER_READ_OOB, index);
        return m_buffer[index];
    }
};
