#include <ArduinoBLE.h>

// byte buffer = 0x0A;
const int BUFFER_SIZE = 5;
const int SIZE_IN_BYTES = BUFFER_SIZE * sizeof(byte);
byte buffer[BUFFER_SIZE] = {1, 2, 3, 4, 5};

float floatArray[] = {1.0, 2.0, 3.0, 4.0, 5.0};
const int arrayLength = sizeof(floatArray) / sizeof(float);
const int byteArraySize = arrayLength * sizeof(float);
byte byteArray[byteArraySize] = {0};

BLEService service1("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// BLEByteCharacteristic characteristic1("weights", BLERead | BLEWrite | BLEBroadcast | BLENotify);
// BLEByteCharacteristic characteristic1("2A37", BLERead | BLEWrite | BLEBroadcast | BLENotify);
BLECharacteristic characteristic1("2A37", BLERead | BLEWrite | BLEBroadcast | BLENotify, byteArraySize);


void floatArrayToByteArray(float* floatArray, byte* byteArray, int arrayLength) {
  // Use memcpy to copy the float array to the byte array
  memcpy(byteArray, floatArray, arrayLength * sizeof(float));
}

void byteArrayToFloatArray(byte* byteArray, float* floatArray, int arrayLength) {
  // Use memcpy to copy the byte array to the float array
  memcpy(floatArray, byteArray, arrayLength * sizeof(float));
}

void setup()
{
    floatArrayToByteArray(floatArray, byteArray, arrayLength);
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
    // characteristic1.writeValue(buffer);
    // characteristic1.writeValue(buffer, SIZE_IN_BYTES);
    characteristic1.writeValue(byteArray, byteArraySize);

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
        // characteristic1.writeValue(buffer);
        // characteristic1.writeValue(buffer, SIZE_IN_BYTES);
        characteristic1.writeValue(byteArray, byteArraySize);
        delay(1000);
      }
    }

}