#pragma once

#define DEBUG

#define ERROR_INVALID_CHECKSUM      "M01"
#define ERROR_UNEXPECTED_MESSAGE    "M02"
#define ERROR_UNEXPECTED_REQUEST    "M03"

#define ERROR_BUFFER_FULL           "B01"
#define ERROR_BUFFER_READ_OOB       "B02"

void halt(const char* message, uint8_t code);

#if defined(DEBUG)
#define ASSERT(e, m, c) do { if (!(e)) halt(m, c); } while(false);
#else
#define ASSERT(e, m)
#endif
