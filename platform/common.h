#pragma once
#include "logging.h"
#include <Arduino.h>

// Define RBOD (Red Blink Of Death) codes
enum RBOD {
    CAMERA_ERROR = 1,
    MEMORY_ERROR,
};

void die(uint8_t strobes);
void *memalloc(size_t sz);