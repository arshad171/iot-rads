#include <ArduinoBLE.h>
byte value = 0;

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

    Serial.println("Discovered characteristics:");
    Serial.print(peripheral.characteristicCount());

    for (int i = 0; i < peripheral.characteristicCount(); i++)
    {
        BLECharacteristic discoveredCharacteristic = peripheral.characteristic(i);
        Serial.print("Characteristic UUID: ");
        Serial.println(discoveredCharacteristic.uuid());
    }
    // retrieve the LED characteristic
    BLECharacteristic readCharacteristic = peripheral.characteristic("2A37");

    if (!readCharacteristic)
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
        readCharacteristic.readValue(value);
        Serial.print("Characteristic read: ");
        Serial.print(value);
        Serial.println();

    }

    Serial.println("Peripheral disconnected");
}
