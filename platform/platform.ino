#include "src/utils/common.h"
#include "src/camera/camera.h"
#include "src/dnn/network.h"

// Communication stack
#include "src/communication/serial/serial.h"
#include "src/communication/protocol.h"

// Platform settings
#include "settings/device.h"

#include "src/dnn/distributed/distributed.h"

enum State {
    SYNC_WEIGHTS,
    DO_TRAINING,
    TRAINING_COMPLETE,
    AWAITING_TRAINING_VECTOR
};


// Runtime variables
static RichMatrix *feature_vector = nullptr;
static BLERole role = BLERole::PERIPHERAL;
static State state = State::SYNC_WEIGHTS;
static int patience;

// Interrupt signals
volatile static bool acquireImage = false;

// Interrupt handlers
void handleShutterButton() {
    acquireImage = true;
}


// Utility functions
void send_picture(Cmd cmd) {
    LOG_SHORT(LOG_DEBUG,"Acquiring image...");
    RichImage *image = get_image();
    pack((byte *) image,getRichImageSize(image),DType::IMG,cmd,&SP);
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

    // Initialize BLE stack
    if(role == BLERole::CENTRAL) {
        BLE_CENTRAL::central_setup();
    } else if(role == BLERole::PERIPHERAL) {
        BLE_PERIPHERAL::setup_peripheral();
    }

    // Initialize autoencoder
    initialize_network();
}

void loop() {
    // If we are a BLE Central we check whether the peripheral wants to speak to us
    if(role == BLERole::CENTRAL) {
        //if(state == State::SYNC_WEIGHTS) {
        //    LOG_SHORT(LOG_INFO,"Performing weight synchronization");
        //    if(!initialized) {
        //        LOG_SHORT(LOG_INFO,"Initializing BLE synchronization");
        //        BLE_CENTRAL::peripheral = BLE.available();
        //        BLE_CENTRAL::BLEserviceConnect();
        //        BLE_CENTRAL::readCharacteristic = BLE_CENTRAL::peripheral.characteristic("2A36");
        //        BLE_CENTRAL::writeCharacteristic = BLE_CENTRAL::peripheral.characteristic("2A37");

        //        if (BLE_CENTRAL::readCharacteristic.subscribe()) {
        //            LOG_SHORT(LOG_INFO, "BLE::subscribed to read characteristics");
        //        }

        //        initialized = BLE_CENTRAL::peripheral.connected();
        //        if(initialized) {
        //            LOG_SHORT(LOG_DEBUG,"BLE::connection established");
        //        }
        //    }

        //    while(initialized) {
        //        if(BLE_CENTRAL::readCharacteristic.valueUpdated()) {
        //            LOG_SHORT(LOG_DEBUG,"BLE::received update event");
        //            BLE_CENTRAL::receive();
        //            BLE_CENTRAL::send();
        //            delay(10);
        //        }

        //        if(BLE_CENTRAL::stopFlag) {
        //            // We must now do training
        //            LOG_SHORT(LOG_INFO,"Weight synchronization complete");
        //            state = State::DO_TRAINING;

        //            begin_training();
        //            // BLE_CENTRAL::peripheral.disconnect();
        //            break;
        //        }
        //    }

        //    // Resume scanning for the peripheral
        //    delay(10);
        //    BLE.scanForUuid("19b10011-e8f2-537e-4f6c-d104768a1214");
        //    delay(10);
        //} else if(state == State::TRAINING_COMPLETE) {
        //    LOG_SHORT(LOG_INFO,"Training complete (loss is %f)",get_training_loss());

        //    // Reset stop flags
        //    BLE_CENTRAL::stopFlag = false;
        //    BLE_CENTRAL::receiveWeightsBuffer.stopFlag = false;
        //    BLE_CENTRAL::peripheral.connect();

        //    // Perform weight synchronization
        //    state = State::SYNC_WEIGHTS;
        //}
        // check if a peripheral has been discovered
        BLE_CENTRAL::peripheral = BLE.available();
        BLE_CENTRAL::BLEserviceConnect();
        BLE_CENTRAL::readCharacteristic = BLE_CENTRAL::peripheral.characteristic("2A36");
        BLE_CENTRAL::writeCharacteristic = BLE_CENTRAL::peripheral.characteristic("2A37");

        if (BLE_CENTRAL::readCharacteristic.subscribe()) {
          LOG_SHORT(LOG_INFO, "BLE::subscribed to read characteristics");
        }

        BLE_CENTRAL::value = BLE_CENTRAL::peripheral.connected();
        while (BLE_CENTRAL::value) {
            if (BLE_CENTRAL::readCharacteristic.valueUpdated()) {
                BLE_CENTRAL::receive();
                BLE_CENTRAL::send();
                delay(10);
            }
            if (BLE_CENTRAL::stopFlag) {
                if (BLE_CENTRAL::trainflag == 0) {
                    blocking_train();
                    BLE_CENTRAL::stopFlag = false;
                    BLE_CENTRAL::receiveWeightsBuffer.stopFlag = false;
                }
                BLE_CENTRAL::trainflag += 1;
                if (BLE_CENTRAL::trainflag > 1) {
                  BLE_CENTRAL::trainflag = 0;
                }
            }
        }

        delay(10);
        BLE.scanForUuid("19b10011-e8f2-537e-4f6c-d104768a1214");
        delay(10);
    } else if(role == BLERole::PERIPHERAL) {
        while(state == State::SYNC_WEIGHTS) {
            if(!BLE_PERIPHERAL::stopFlag) {
                LOG_SHORT(LOG_DEBUG,"Searching for paired devices (send=%d)",BLE_PERIPHERAL::sendFlag);
                BLE.poll(10000);
                delay(100);

                if(BLE_PERIPHERAL::sendFlag) {
                    // Attempt to send a sync request to a hypothetical central
                    BLE_PERIPHERAL::sendSyncFlag();
                    LOG_SHORT(LOG_DEBUG,"Exited sendSyncFlag");
                }

                // If the central has sent back weights we have a connection
                if(BLE_PERIPHERAL::writeCharacteristic.written()) {
                    LOG_SHORT(LOG_DEBUG,"Central has connected, initiating sync...");
                    BLE_PERIPHERAL::sendFlag = false;
                    BLE_PERIPHERAL::receive();
                    BLE_PERIPHERAL::send();
                    delay(10);
                }
            } else {
                // We have received the stop flag: weight sync is complete, now train
                LOG_SHORT(LOG_INFO,"Weight synchronization complete");
                state = State::DO_TRAINING;

                begin_training();
            }
        }
        
        if(state == State::TRAINING_COMPLETE) {
            // We have completed training: reset the stop flag
            LOG_SHORT(LOG_INFO,"Training complete (loss is %f)",get_training_loss());

            BLE_PERIPHERAL::stopFlag = false;
            state = State::SYNC_WEIGHTS;
        }
    }

    // Are we in a deadlock?
    if(state == State::AWAITING_TRAINING_VECTOR && patience-- <= 0) {
        state = State::DO_TRAINING;
    }

    // Handle request of training data
    if(state == State::DO_TRAINING) {
        if(!DNN_DEBUG) {
            LOG_SHORT(LOG_INFO,"Requesting feature vector...");
            pack(nullptr,0,DType::CMD,Cmd::GET_FEATURE_VECTOR,&SP);
            state = State::AWAITING_TRAINING_VECTOR;
            patience = PATIENCE;
        } else {
            RichMatrix *fake_features = (RichMatrix *) memalloc(getRichMatrixSize(FEATURE_DIM,1,MatrixType::TYPE_FLOAT32));
            fake_features->metadata.rows = FEATURE_DIM;
            fake_features->metadata.cols = 1;
            fake_features->metadata.type = MatrixType::TYPE_FLOAT32;

            float *data_ptr = (float *) fake_features->data;
            for(int i=0; i < FEATURE_DIM; i++) {
                data_ptr[i] = float(random(-1000, 1000) / 1000.0);
            }

            LOG_SHORT(LOG_DEBUG,"Processing fake feature vector for S P E E D");
            bool fake_train_complete = process_feature(fake_features);
            free(fake_features);

            if(fake_train_complete) {
                LOG_SHORT(LOG_DEBUG,"Fake training complete!");
                state = State::TRAINING_COMPLETE;
            }
        }
    }

    // Read incoming commands
    while(true) {
        Packet incoming = SP.recv();
        bool should_free = true;

        // Stop when we process all incoming packets
        if(incoming.header.magic[0] == 0) {
            break;
        }

        LOG_SHORT(LOG_DEBUG,"Received packet with %d byte payload",incoming.header.size);

        switch(incoming.header.command) {
            case Cmd::GET_FRAME:
                send_picture(Cmd::SET_FRAME);
                break;

            case Cmd::GET_BROLL: {
                byte count = *((byte *) incoming.data);
                for(int i=0;i<count;i++)
                    send_picture(Cmd::SET_BROLL);
                    break;
            }

            // We're doing training Jesse
            case Cmd::SET_TRAINING_VECTOR: {
                if(incoming.header.type != DType::MAT) {
                    LOG(LOG_ERROR,"Received feature vector of wrong type %d",incoming.header.type);
                    break;
                }

                if(state == State::AWAITING_TRAINING_VECTOR) {
                    state == State::DO_TRAINING;
                }

                if(feature_vector != nullptr) {
                    free(feature_vector);
                    LOG_SHORT(LOG_DEBUG,"Old feature vector discarded");
                }

                feature_vector = (RichMatrix *) incoming.data;
                const uint16_t r = feature_vector->metadata.rows;
                const uint16_t c = feature_vector->metadata.cols;
                LOG_SHORT(LOG_DEBUG,"Received %dx%d feature vector (%d)",r,c,r*c);

                // Pass the feature vector to training code
                bool training_finished = process_feature(feature_vector);
                if(training_finished) {
                    state = State::TRAINING_COMPLETE;
                }

                should_free = false; // DO NOT FREE! The training data stays until replaced
                break;
            }

            case Cmd::GET_WEIGHTS: {
                LOG_SHORT(LOG_DEBUG,"serving GET_WEIGHTS request");
                send_layer_weights(1);
                send_layer_weights(2);
                send_layer_weights(3);
                send_layer_weights(4);
                break;
            }
            // We're doing inference Waltuh
            case Cmd::SET_INFERENCE_VECTOR: {
                if(incoming.header.type != DType::MAT) {
                    LOG(LOG_ERROR,"Received feature vector of wrong type %d",incoming.header.type);
                    break;
                }

                RichMatrix *vector = (RichMatrix *) incoming.data;
                const uint16_t r = vector->metadata.rows;
                const uint16_t c = vector->metadata.cols;
                LOG_SHORT(LOG_DEBUG,"Received %dx%d inference vector (%d)",r,c,r*c);

                if(r != FEATURE_DIM || c != 1) {
                    LOG_SHORT(LOG_ERROR,"Inference vector has wrong dimension");
                    break;
                }

                // Allocate the prediction vector
                float *data = (float *) vector->data;
                BLA::Matrix<FEATURE_DIM,BATCH_SIZE> prediction_vector;
                for(int i=0; i < r; i++) {
                    prediction_vector(i,0) = data[i];
                    prediction_vector(i,1) = data[i];
                }

                // Pass the feature vector to training code
                float loss = predict(prediction_vector);
                LOG_SHORT(LOG_INFO,"Prediction loss: %f", loss);
                break;
            }

            // We requested a feature vector but none was available
            case Cmd::NO_FEATURE_VECTOR:
                LOG_SHORT(LOG_WARNING,"No feature vector available, retrying later...");
                patience = PATIENCE;
                break;

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
        send_picture(Cmd::SET_FRAME);

        // Reset the handler
        acquireImage = false;
        attachInterrupt(digitalPinToInterrupt(D13),&handleShutterButton,FALLING);
    }
}