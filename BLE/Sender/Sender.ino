#include <ArduinoBLE.h>

byte sendBuffer[100] = {0};

BLEService service1("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// BLEByteCharacteristic characteristic1("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLECharacteristic characteristic1("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, sizeof(sendBuffer));

// float sendBuffer = 1.0;

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;

    // begin initialization
    if (!BLE.begin())
    {
        Serial.println("starting Bluetooth® Low Energy module failed!");

        while (1)
            ;
    }

    // set advertised local name and service UUID:
    BLE.setLocalName("sender");
    BLE.setAdvertisedService(service1);

    // add the characteristic to the service
    service1.addCharacteristic(characteristic1);

    // add service
    BLE.addService(service1);

    // set the initial value for the characeristic:
    // characteristic1.writeValue(0);
    // characteristic1.writeValue(*(byte *) sendBuffer);
    characteristic1.writeValue(sendBuffer, sizeof(sendBuffer));

    // start advertising
    BLE.advertise();

    Serial.println("i am the sender...");
}

void loop()
{
    Serial.println("in the loop");
    delay(1000);
}