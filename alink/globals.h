#pragma once
#include <stdint.h>
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "errors.h"
#include "utils.h"

#define VERSION "1.07"
#define VERSION_HEX 0x6B
#define DISPLAY_LINE_LENGTH 16
#define DATA_BUFFER_SIZE 16
#define PORT_SPEED 115200

typedef uint8_t Handle;
typedef void (*pfnMode)(void);

extern LiquidCrystal lcd;