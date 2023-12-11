#include "src/utils/common.h"
#include "src/camera/camera.h"
#include "src/dnn/network.h"

// Communication stack
#include "src/communication/serial/serial.h"
#include "src/communication/protocol.h"

// Platform settings
#include "settings/device.h"

// Interrupt signals
volatile static bool acquireImage = false;

// Interrupt handlers
void handleShutterButton() {
    acquireImage = true;
}

// Utility functions
void send_picture() {
    LOG_SHORT(LOG_DEBUG,"Acquiring image...");
    RichImage *image = get_image();
    pack((byte *) image,getRichImageSize(image),DType::IMG,Cmd::SET_FRAME,&SP);
    free(image);
}



void setup() {
    // Setup the board's status indicators
    PINMODE(LEDR,OUTPUT); PINMODE(LEDG,OUTPUT); PINMODE(LEDB,OUTPUT); PINMODE(LED_PWR,OUTPUT);
    DWRITE(LEDR,1); DWRITE(LEDG,1); DWRITE(LEDB,1); DWRITE(LED_PWR,0);

    // Setup PIN 13 (The PushButton) to input
    PINMODE(D13,INPUT_PULLUP);

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

    // Setup camera
    camera_init(CAMERA_MODE,CAMERA_COLOR,CAMERA_FPS);

    // Setup interrupts
    attachInterrupt(digitalPinToInterrupt(D13),&handleShutterButton,FALLING);

    // Tell the user we completed setup
    DWRITE(LED_PWR,1);

    // Do stuff
    initialize();
}

// Main loop of the program
void loop() {
    // Read incoming commands
    while(true) {
        Packet incoming = SP.recv();

        // Stop when we process all commands
        if(incoming.header.magic[0] == 0) {
            break;
        }
        
        switch(incoming.header.command) {
            case Cmd::GET_FRAME:
                LOG_SHORT(LOG_INFO,(char *) incoming.data);
                send_picture();
                break;
        }

        // Free the data after we are done
        if(incoming.data != nullptr) {
            free(incoming.data);
        }
    }
    

    train();

    if(acquireImage) {
        send_picture();        

        // Reset the handler
        acquireImage = false;
        attachInterrupt(digitalPinToInterrupt(D13),&handleShutterButton,FALLING);
    }
}