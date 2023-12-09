#include <ArduinoBLE.h>

// byte buffer = 0x0A;
const int BUFFER_SIZE = 5;
const int SIZE_IN_BYTES = BUFFER_SIZE * sizeof(byte);
byte buffer[BUFFER_SIZE] = {1, 2, 3, 4, 5};

float floatArray[] = {1.0, 2.0, 3.0, 4.0, 5.0};
const int arrayLength = sizeof(floatArray) / sizeof(float);
const int byteArraySize = arrayLength * sizeof(float);
byte byteArray[byteArraySize] = {0};

int flag = 1;

BLEService networkService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

BLECharacteristic readCharacteristic("2A36", BLERead | BLEIndicate,byteArraySize);
BLECharacteristic writeCharacteristic("2A37", BLEWrite, byteArraySize);


void floatArrayToByteArray(float* floatArray, byte* byteArray, int arrayLength) {
  // Use memcpy to copy the float array to the byte array
  memcpy(byteArray, floatArray, arrayLength * sizeof(float));
}

void byteArrayToFloatArray(byte* byteArray, float* floatArray, int arrayLength) {
  // Use memcpy to copy the byte array to the float array
  memcpy(floatArray, byteArray, arrayLength * sizeof(float));
}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  BLE.advertise();
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  BLE.advertise();
  flag = 1;
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
    BLE.setAdvertisedService(networkService);

    // add the characteristic to the service
    networkService.addCharacteristic(readCharacteristic);
    networkService.addCharacteristic(writeCharacteristic);

    // add service
    BLE.addService(networkService);

    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);


    // set the initial value for the characeristic:
    // characteristic1.writeValue(buffer);
    // characteristic1.writeValue(buffer, SIZE_IN_BYTES);
    readCharacteristic.writeValue(byteArray, byteArraySize);

    // start advertising
    BLE.advertise();

    Serial.println("i am the sender...");
}

void loop()
{
  BLE.poll();
  if (flag == 1){

      readCharacteristic.writeValue(byteArray, byteArraySize);
      Serial.println("Sending DATA!!!");

  }

  if (writeCharacteristic.written()){
      flag = 0;
      writeCharacteristic.readValue( (byte*)&floatArray, byteArraySize);

      Serial.println("My float Values:");
      Serial.println(floatArray[0]);
      Serial.println(floatArray[1]);
      Serial.println(floatArray[2]);    

      floatArray[0] = floatArray[0] + 0.5;

      readCharacteristic.writeValue((byte*)&floatArray,byteArraySize);

  }

}