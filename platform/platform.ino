#include "src/utils/common.h"
#include "src/camera/camera.h"
#include "src/dnn/network.h"

// Communication stack
#include "src/communication/serial/serial.h"
#include "src/communication/protocol.h"

// Platform settings
#include "settings/device.h"



// Runtime variables
static RichMatrix *feature_vector = nullptr;



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


void request_feature_vector() {
  LOG_SHORT(LOG_DEBUG, "Requesting feature vector...");
  pack(nullptr, 0, DType::CMD, Cmd::GET_FEATURE_VECTOR, &SP);
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
        bool should_free = true;

        // Stop when we process all commands
        if(incoming.header.magic[0] == 0) {
            break;
        }

        LOG_SHORT(LOG_DEBUG,"Received packet with %d byte payload",incoming.header.size);
        
        switch(incoming.header.command) {
            case Cmd::GET_FRAME:
                send_picture();
                break;
            
            case Cmd::SET_FEATURE_VECTOR: {
                if(incoming.header.type != DType::MAT) {
                    LOG(LOG_ERROR,"Received feature vector of wrong type %d",incoming.header.type);
                    break;
                }


                LOG_SHORT(LOG_DEBUG,"Received %dx%d feature vector",feature_vector->metadata.rows,feature_vector->metadata.cols);
                if(feature_vector != nullptr && should_free) {
                    free(feature_vector);
                    LOG_SHORT(LOG_DEBUG,"Old feature vector discarded");
                }

                feature_vector = (RichMatrix *) incoming.data;

                const int size = feature_vector->metadata.rows * feature_vector->metadata.cols;

                LOG_SHORT(LOG_INFO, "size: %d", size);

                float features[size];

                memcpy(features, feature_vector->data, size * sizeof(float));

                LOG_SHORT(LOG_INFO, "*features: %f %f", features[0], features[1]);

                should_free = false;

                // Do the training
                train();

                // just for testting. this should ideally go in the train function
                request_feature_vector();

                // free(features);
                break;
              }
            default:
                LOG_SHORT(LOG_WARNING,"Received unknown command %d",incoming.header.command);
                break;
        }

        // Free the data after we are done
        if(incoming.data != nullptr && should_free) {
            LOG_SHORT(LOG_DEBUG,"Freeing incoming data");
            free(incoming.data);
        }

    }

    // Handle interrupts
    if(acquireImage) {
        send_picture();        

        // Reset the handler
        acquireImage = false;
        attachInterrupt(digitalPinToInterrupt(D13),&handleShutterButton,FALLING);
    }
}