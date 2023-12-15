#pragma once
#include "logging.h"
#include <Arduino.h>

// Define convenience macros
#define PINMODE(p,m) arduino::pinMode(p,m)
#define DWRITE(p,v) arduino::digitalWrite(p,v)

// Define RBOD (Red Blink Of Death) codes
enum RBOD {
    CAMERA_ERROR = 1,
    MEMORY_ERROR,
    FATAL_PROTOCOL_ERROR,
    FATAL_BLE_ERROR
};

void die(uint8_t strobes);
void *memalloc(size_t sz);