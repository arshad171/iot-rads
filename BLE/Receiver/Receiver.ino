#include <ArduinoBLE.h>
byte value = 0;
byte Byte_Array[100] = {0};
float Float_Array[100] = {0.0};
int buffer_size = 0;

/*void byteArrayToFloatArray(byte *byteArray, float *floatArray, int size) {
  for (int i = 0; i < size; i++) {
    memcpy(&floatArray[i], &byteArray[i * sizeof(float)], sizeof(float));
  }
}*/

void byteArrayToFloatArray(byte* byteArray, float* floatArray, int arrayLength) {
  // Use memcpy to copy the byte array to the float array
  memcpy(floatArray, byteArray, arrayLength * sizeof(float));
}




/*void printFloatArray(float *floatArray, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(floatArray[i], 2); // Print with 2 decimal places
    Serial.print(" ");
  }
  Serial.println();
}

void printByteArray(byte *byteArray, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(byteArray[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}*/

void exploreCharacteristic(BLECharacteristic characteristic) {
  // print the UUID and properties of the characteristic
  Serial.print("\tCharacteristic ");
  Serial.print(characteristic.uuid());
  Serial.print(", properties 0x");
  Serial.print(characteristic.properties(), HEX);

  // check if the characteristic is readable
  if (characteristic.canRead()) {
    // read the characteristic value
    characteristic.read();

    if (characteristic.valueLength() > 0) {
      // print out the value of the characteristic
      Serial.println(", value 0x");
      //printData(characteristic.value(), characteristic.valueLength());
      buffer_size = characteristic.valueLength();
      characteristic.readValue(Byte_Array,100);
      byteArrayToFloatArray(Byte_Array,Float_Array,100 );

      Serial.println("My float values:  ");
      Serial.println(Float_Array[0]);
      Serial.println(Float_Array[1]);
      Serial.println(Float_Array[2]);
    }
  }
  Serial.println();

  // loop the descriptors of the characteristic and explore each
  for (int i = 0; i < characteristic.descriptorCount(); i++) {
    BLEDescriptor descriptor = characteristic.descriptor(i);

    exploreDescriptor(descriptor);
  }
}

void exploreDescriptor(BLEDescriptor descriptor) {
  // print the UUID of the descriptor
  Serial.print("\t\tDescriptor ");
  Serial.print(descriptor.uuid());

  // read the descriptor value
  descriptor.read();

  // print out the value of the descriptor
  Serial.print(", value 0x");
  printData(descriptor.value(), descriptor.valueLength());

  Serial.println();
}

void printData(const unsigned char data[], int length) {
  for (int i = 0; i < length; i++) {
    unsigned char b = data[i];

    if (b < 16) {
      Serial.print("0");
    }

    Serial.print(b, HEX);
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

void loop()
{
    // check if a peripheral has been discovered
    BLEDevice peripheral = BLE.available();

    if (peripheral)
    {
        // discovered a peripheral, print out address, local name, and advertised service
        Serial.print("Found ");
        Serial.print(peripheral.address());
        Serial.print(" '");
        Serial.print(peripheral.localName());
        Serial.print("' ");
        Serial.print(peripheral.advertisedServiceUuid());
        Serial.println();

        if (peripheral.localName() != "sender")
        {
            return;
        }

        // stop scanning
        BLE.stopScan();

        controlLed(peripheral);

        // peripheral disconnected, start scanning again
        BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
        //BLE.scanForUuid("2A37");
    }
}

void controlLed(BLEDevice peripheral)
{
    // connect to the peripheral
    Serial.println("Connecting ...");

    if (peripheral.connect())
    {
        Serial.println("Connected");
    }
    else
    {
        Serial.println("Failed to connect!");
        return;
    }

    // discover peripheral attributes
    Serial.println("Discovering attributes ...");
    if (peripheral.discoverAttributes())
    {
        Serial.println("Attributes discovered");
    }
    else
    {
        Serial.println("Attribute discovery failed!");
        peripheral.disconnect();
        return;
    }

    /*Serial.println("Discovered characteristics:");
    Serial.print(peripheral.characteristicCount());

    for (int i = 0; i < peripheral.characteristicCount(); i++)
    {
        BLECharacteristic discoveredCharacteristic = peripheral.characteristic(i);
        Serial.print("Characteristic UUID: ");
        Serial.println(discoveredCharacteristic.uuid());
    }*/

    // retrieve the LED characteristic
    BLECharacteristic readCharacteristic = peripheral.characteristic("2A37");
    exploreCharacteristic(readCharacteristic);

    /*if (!readCharacteristic)
    {
        Serial.println("Peripheral does not have LED characteristic!");
        peripheral.disconnect();
        return;
    }
    else if (!readCharacteristic.canRead())
    {
        Serial.println("Peripheral does not have a writable LED characteristic!");
        peripheral.disconnect();
        return;
    }

    while (peripheral.connected())
    {
        /*readCharacteristic.readValue(value);
        Serial.print("Characteristic read: ");
        Serial.print(value);
        Serial.println();*/

    //} */

    //Serial.println("Peripheral disconnected");
}
