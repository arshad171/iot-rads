#include <ArduinoBLE.h>

byte buffer = 0x0A;
// byte buffer[100] = {0};

BLEService service1("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// BLEByteCharacteristic characteristic1("weights", BLERead | BLEWrite | BLEBroadcast | BLENotify);
BLEByteCharacteristic characteristic1("2A37", BLERead | BLEWrite | BLEBroadcast | BLENotify);
// BLECharacteristic characteristic1("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, sizeof(sendBuffer));

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
    characteristic1.writeValue(buffer);

    // start advertising
    BLE.advertise();

    Serial.println("i am the sender...");
}

void loop()
{
    BLEDevice central = BLE.central();

    if (central) {
      while(central.connect()) {
        Serial.println("sending data");
        characteristic1.writeValue(buffer);
        // delay(1000);
      }
    }

}