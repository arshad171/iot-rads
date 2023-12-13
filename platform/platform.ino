#include "src/utils/common.h"
#include "src/camera/camera.h"
#include "src/dnn/network.h"

// Communication stack
#include "src/communication/serial/serial.h"
#include "src/communication/protocol.h"

// Platform settings
#include "settings/device.h"

#include "src/dnn/distributed/central.h"
#include "src/dnn/distributed/peripheral.h"



// Runtime variables
static RichMatrix *feature_vector = nullptr;
static bool must_request_data = true;
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

void request_feature_vector() {
  LOG_SHORT(LOG_DEBUG, "Requesting feature vector...");
  pack(nullptr, 0, DType::CMD, Cmd::GET_FEATURE_VECTOR, &SP);
}

bool receive_feature_vector(int batchIndex)
{
  bool success = false;
  bool awaitResponse = true;

  for (int maxIters = 10; (maxIters > 0) && (awaitResponse); maxIters--)
  {
    Packet incoming = SP.recv();
    bool should_free = true;

    // // Stop when we process all incoming packets
    if (incoming.header.magic[0] == 0)
    {
      continue;
    }

    LOG_SHORT(LOG_DEBUG, "Received packet with %d byte payload", incoming.header.size);

    switch (incoming.header.command)
    {
    case Cmd::SET_FEATURE_VECTOR:
    {
      if (incoming.header.type != DType::MAT)
      {
        LOG(LOG_ERROR, "Received feature vector of wrong type %d", incoming.header.type);
        break;
      }

      LOG_SHORT(LOG_DEBUG, "Received %dx%d feature vector", feature_vector->metadata.rows, feature_vector->metadata.cols);
      if (feature_vector != nullptr && should_free)
      {
        free(feature_vector);
        LOG_SHORT(LOG_DEBUG, "Old feature vector discarded");
      }

      feature_vector = (RichMatrix *)incoming.data;

      const int size = feature_vector->metadata.rows * feature_vector->metadata.cols;

      LOG_SHORT(LOG_INFO, "size: %d", size);

      float features[size] = {0.0};

      memcpy(features, feature_vector->data, size * sizeof(float));

      for (int r = 0; r < xBatch.Rows; r++)
      {
        xBatch(r, batchIndex) = features[r];
      }

      should_free = false; // DO NOT FREE! The training data stays until replaced
      awaitResponse = false;
      success = true;
      break;
    }
    }
  }
  return success;
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
    PERIPHERAL::setup_peripheral();
    initialize();
}



// Main loop of the program
void central_loop()
{
  using namespace CENTRAL;

  // check if a peripheral has been discovered
  peripheral = BLE.available();
  BLEserviceConnect();
  readCharacteristic = peripheral.characteristic("2A36");
  writeCharacteristic = peripheral.characteristic("2A37");

  if (readCharacteristic.subscribe())
  {
    LOG_SHORT(LOG_INFO, "BLE::subscribed to read characteristics");
  }

  value = peripheral.connected();
  while (value)
  {
    if (readCharacteristic.valueUpdated())
    {
      receive();
      send();
      delay(10);
    }
    if (stopFlag)
    {

      if (trainflag == 0)
      {
        train();
        stopFlag = false;
        receiveWeightsBuffer.stopFlag = false;
      }
      trainflag += 1;
      if (trainflag > 1)
      {
        trainflag = 0;
      }
    }
  }
  delay(10);
  BLE.scanForUuid("19b10011-e8f2-537e-4f6c-d104768a1214");
  delay(10);
}

void peripheral_loop()
{
  using namespace PERIPHERAL;

  if (!stopFlag)
  {
    BLE.poll();
    if (sendFlag)
    {
      sendSyncFlag();
    }

    if (writeCharacteristic.written())
    {
      sendFlag = false;

      receive();
      send();

      delay(10);
    }
  }
  else
  {
    train();
    stopFlag = false;
  }
}

void loop() {
    peripheral_loop();
    return;

    // Decrease patience at every iteration to avoid deadlocks
    patience--;

    if(must_request_data == false && patience < 1) {
        // Patience ran out: request data again
        must_request_data = true;
    }

    // Request data if necessary
    if(must_request_data) {
        LOG_SHORT(LOG_INFO,"Requesting feature vector...");
        pack(nullptr,0,DType::CMD,Cmd::GET_FEATURE_VECTOR,&SP);

        // Wait for response up to a given amount of attempts
        must_request_data = false;
        patience = PATIENCE;
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

            case Cmd::SET_FEATURE_VECTOR: {
                if(incoming.header.type != DType::MAT) {
                    LOG(LOG_ERROR,"Received feature vector of wrong type %d",incoming.header.type);
                    break;
                }

                LOG_SHORT(LOG_DEBUG,"Received %dx%d feature vector",feature_vector->metadata.rows,feature_vector->metadata.cols);
                if(feature_vector != nullptr) {
                    free(feature_vector);
                    LOG_SHORT(LOG_DEBUG,"Old feature vector discarded");
                }

                feature_vector = (RichMatrix *) incoming.data;
                const int size = feature_vector->metadata.rows * feature_vector->metadata.cols;

                LOG_SHORT(LOG_INFO, "size: %d", size);

                float features[size];

                memcpy(features, feature_vector->data, size * sizeof(float));

                LOG_SHORT(LOG_INFO, "*features: %f %f", features[0], features[1]);

                // Do the training
                train();

                should_free = false; // DO NOT FREE! The training data stays until replaced
                must_request_data = true; // We processed this batch; request the next
                break;
            }

            // We requested a feature vector but none was available _> reset patience
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