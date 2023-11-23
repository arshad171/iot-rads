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
#include <BasicLinearAlgebra.h>

void setup() {
    // Setup the board's status indicators
    pinMode(LEDR,OUTPUT); pinMode(LEDG,OUTPUT); pinMode(LEDB,OUTPUT); pinMode(LED_PWR,OUTPUT);
    digitalWrite(LEDR,1); digitalWrite(LEDG,1); digitalWrite(LEDB,1); digitalWrite(LED_PWR,0);

    // Initialize serial port
    SP.initialize(9600,100);

    // Setup logging
    set_log_lvl(LOG_LEVEL);
    toggle_logs(true);

    // Wait on the serial and notify user
    digitalWrite(LEDB,0);
    SP.blocking_wait(0);
    digitalWrite(LEDB,1);

    // Greet the user and present hardware info
    LOG_SHORT(LOG_INFO,"Railway Anomaly Detection System v%s",FW_VERSION);

    // Initialize more hardware
    camera_init(CAMERA_MODE,CAMERA_COLOR,CAMERA_FPS);

    // Tell the user we completed setup
    digitalWrite(LED_PWR,1);

    // Do stuff
    initialize();
}

void loop(){
    train();
}