#pragma once

#if defined(DEBUG)
#define debugDelay() delay(500)
#define debugMessage(m) display(1, m)
#else
#define debugDelay(...)
#define debugMessage(...)
#endif

void display(uint8_t line, const char* message);
char nibbleToHex(uint8_t nibble);
uint8_t writeMessage(uint8_t byte, uint8_t checkSum = 0);