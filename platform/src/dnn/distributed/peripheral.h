#pragma once
#include <ArduinoBLE.h>

namespace PERIPHERAL {
    extern bool sendFlag;
    extern bool stopFlag;
    extern BLECharacteristic writeCharacteristic;

    void setup_peripheral();
    void send();
    void receive();
    void sendSyncFlag();
}