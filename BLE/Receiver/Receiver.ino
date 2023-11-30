#include <ArduinoBLE.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();
  
  Serial.println("Started Scan.  Testing waiting 0.1 minute");
  Serial.println();

  delay(6000);
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  // Scan only for ATC_ devices
  if (peripheral && peripheral.hasLocalName() && peripheral.localName().startsWith("Sender")) {
    // discovered a peripheral
    Serial.println("Discovered a peripheral");
    Serial.println("-----------------------");

    // print address
    Serial.print("Address: ");
    Serial.println(peripheral.address());


    // print the local name, if present
    if (peripheral.hasLocalName()) {
      Serial.print("Local Name: ");
      Serial.println(peripheral.localName());
    }

    uint8_t advertisement[64] = {0};
    int adLength = peripheral.getAdvertisement(advertisement,64);
    Serial.print("Advertisement: 0x");
    Serial.print(advertisement[0]);
    Serial.print(advertisement[1]);
    Serial.print(advertisement[2]);
    Serial.print(advertisement[3]);
    Serial.print(advertisement[4]);
    Serial.print(advertisement[5]);
    Serial.print(advertisement[6]);
    Serial.print(advertisement[7]);
    Serial.print(advertisement[8]);
    Serial.print(advertisement[9]);
    Serial.print(advertisement[10]);
    Serial.print(advertisement[11]);
    Serial.print(advertisement[12]);
    Serial.println();
    uint8_t sensorAdvertisementData[64];  // TODO - allocate according to length or spec

   }
   

  
}
