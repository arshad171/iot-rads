#include "common.h"
#include "camera.h"
#include "protocol.h"

// Device information (include experiment name)
#define FW_VERSION "0.0.1-Platform-Beta"

// Communication options
#define BAUD_RATE 9600
#define LOGL_LEVEL LOG_DEBUG

// Camera parameters
#define CAMERA_MODE  QVGA
#define CAMERA_COLOR GRAYSCALE
#define CAMERA_FPS   1

void setup() {
    // Setup the board's status indicators
    pinMode(LEDR,OUTPUT); pinMode(LEDG,OUTPUT); pinMode(LEDB,OUTPUT); pinMode(LED_PWR,OUTPUT);
    digitalWrite(LEDR,1); digitalWrite(LEDG,1); digitalWrite(LEDB,1); digitalWrite(LED_PWR,0);

    // Initialize serial port
    SP.initialize(9600,100);

    // Setup logging
    set_log_lvl(LOGL_LEVEL);
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
}

void loop(){

}