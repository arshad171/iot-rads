#include "common.h"

// Halt execution
void die(uint8_t strobes) {
    uint8_t count = 0;

    // Switch off the RGB led
    digitalWrite(LEDR,1);
    digitalWrite(LEDG,1);
    digitalWrite(LEDB,1);

    // Switch off the power led
    digitalWrite(LED_PWR,0);

    int led_state = 1;
    while(strobes > 0) {
        digitalWrite(LEDR,led_state);

        // Keep track of the strobes
        count += led_state;
        led_state = 1 - led_state;

        // Wait longer between codes
        if(count == strobes && led_state == 0) {
            delay(2000);
            count = 0;
        } else {
            delay(500);
        }
    }

    // If strobes is <= 0 then steady red
    digitalWrite(LEDR,0);
    while(1);
}

// Allocate memory with advanced error reporting
void *memalloc(size_t sz) {
    void *ptr = malloc(sz);
    if(ptr == nullptr) {
        // We can't log anything here or we cause an infinite loop
        die(RBOD::MEMORY_ERROR);
    }
}