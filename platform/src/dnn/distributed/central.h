#pragma once
#include <ArduinoBLE.h>

#define BLE_NUM_WEIGHTS 25

namespace CENTRAL {
  typedef struct __attribute__((packed))
  {
    bool syncFlag = false;
    bool biasFlag = false;
    bool stopFlag = false;
    int layerIndex;
    int rowIndex;
    int colIndex;
    float weights[BLE_NUM_WEIGHTS];
  } WeightsBuffer;

  extern bool value;
  extern bool stopFlag;
  extern int trainflag;
  extern BLEDevice peripheral;
  extern BLECharacteristic writeCharacteristic;
  extern BLECharacteristic readCharacteristic;
  extern WeightsBuffer receiveWeightsBuffer;

  void BLEserviceConnect();
  void receive();
  void send();
  void central_setup();
}