#pragma once

void display(uint8_t line, const char* message);
char nibbleToHex(uint8_t nibble);
uint8_t writePacket(uint8_t byte, uint8_t checkSum = 0);