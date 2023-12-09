#include <ArduinoBLE.h>

float floatArray[] = {1.0, 2.0, 3.0, 4.0, 5.0};
const int arrayLength = sizeof(floatArray) / sizeof(float);
const int byteArraySize = arrayLength * sizeof(float);


byte byteArray[byteArraySize] = {0};

BLEDevice peripheral;
BLECharacteristic writeCharacteristic;
BLECharacteristic readCharacteristic;

void floatArrayToByteArray(float* floatArray, byte* byteArray, int arrayLength) {
  // Use memcpy to copy the float array to the byte array
  memcpy(byteArray, floatArray, arrayLength * sizeof(float));
}

void byteArrayToFloatArray(byte* byteArray, float* floatArray, int arrayLength) {
  // Use memcpy to copy the byte array to the float array
  memcpy(floatArray, byteArray, arrayLength * sizeof(float));
}

void BLEserviceConnect(){

  if (peripheral){
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "sender"){
        return;
    }

    // stop scanning
    BLE.stopScan();

    //controlLed(peripheral);
    peripheral.connect();
    peripheral.discoverAttributes();
  }
}

void BLEserviceExecution(){ 
  readCharacteristic = peripheral.characteristic("2A36");
  writeCharacteristic = peripheral.characteristic("2A37");

  if (readCharacteristic.subscribe()) {
    Serial.println("Subscribed to read characteristic");
  }
  while(peripheral.connected()){
    if (readCharacteristic.valueUpdated()){
      readCharacteristic.readValue((byte*)&floatArray,byteArraySize);
      Serial.println("My float Values:");
      Serial.println(floatArray[0]);
      Serial.println(floatArray[1]);
      Serial.println(floatArray[2]); 

      floatArray[1] +=0.5; 
      writeCharacteristic.writeValue((byte*)&floatArray,byteArraySize);

    }

  }

}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;

    // initialize the Bluetooth® Low Energy hardware
    BLE.begin();

    // start scanning for peripherals
    BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
    //BLE.scanForUuid("2A37");

    Serial.println("i am the receiver...");
}

void loop(){
  // check if a peripheral has been discovered
  peripheral = BLE.available();
  BLEserviceConnect();
  BLEserviceExecution();

      // peripheral disconnected, start scanning again
  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
      //BLE.scanForUuid("2A37");
    
}
