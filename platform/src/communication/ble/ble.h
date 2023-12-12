#include <ArduinoBLE.h>

enum BLERole {
    PERIPHERAL,
    CENTRAL
};

class BLEDeviceRADS {
    private:
        BLEService service;
        BLECharacteristic write_characteristic;
        BLECharacteristic read_characteristic;


    public:
        BLEDeviceRADS(char *name);
};

class BLEPeripheralRADS : public BLEDeviceRADS {
    private:
        BLEDevice central;

        static void on_connect(BLEDevice central);
        static void on_disconnect(BLEDevice central);
    public:
        bool sendFlag = true;

        BLEPeripheralRADS(char *name);
};
