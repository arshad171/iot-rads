#include "common.h"
#include "camera.h"
#include "protocol.h"
#include "network.h"

// Platform settings
#include "settings/device.h"

// Neural network implementation
#include "neural_network/linear_layer.h"
#include "neural_network/relu_layer.h"
#include "neural_network/squared_loss.h"

// External libraries
// #include <BasicLinearAlgebra.h>

void setup() {
    // Setup the board's status indicators
    PINMODE(LEDR,OUTPUT); PINMODE(LEDG,OUTPUT); PINMODE(LEDB,OUTPUT); PINMODE(LED_PWR,OUTPUT);
    DWRITE(LEDR,1); DWRITE(LEDG,1); DWRITE(LEDB,1); DWRITE(LED_PWR,0);

    // Initialize serial port
    SP.initialize(BAUD_RATE,1000);

    // Setup logging
    set_log_lvl(LOG_LEVEL);
    toggle_logs(true);

    // Wait on the serial and notify user
    DWRITE(LEDB,0);
    SP.blocking_wait(0);
    DWRITE(LEDB,1);

    // Greet the user and present hardware info
    LOG_SHORT(LOG_INFO,"Railway Anomaly Detection System v%s",FW_VERSION);

    // Tell the user we completed setup
    DWRITE(LED_PWR,1);

    // Do stuff
    initialize();
}

void loop(){
    train();
}