#include <ArduinoBLE.h>

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;

    // initialize the Bluetooth® Low Energy hardware
    BLE.begin();

    // start scanning for peripherals
    BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");

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

        // controlLed(peripheral);

        // peripheral disconnected, start scanning again
        BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
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

    // retrieve the LED characteristic
    BLECharacteristic ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

    if (!ledCharacteristic)
    {
        Serial.println("Peripheral does not have LED characteristic!");
        peripheral.disconnect();
        return;
    }
    else if (!ledCharacteristic.canWrite())
    {
        Serial.println("Peripheral does not have a writable LED characteristic!");
        peripheral.disconnect();
        return;
    }

    while (peripheral.connected())
    {
        ledCharacteristic.writeValue((byte)0x01);
        // while the peripheral is connected

        // read the button pin
        // int buttonState = digitalRead(buttonPin);

        // if (oldButtonState != buttonState)
        // {
        //     // button changed
        //     oldButtonState = buttonState;

        //     if (buttonState)
        //     {
        //         Serial.println("button pressed");

        //         // button is pressed, write 0x01 to turn the LED on
        //         ledCharacteristic.writeValue((byte)0x01);
        //     }
        //     else
        //     {
        //         Serial.println("button released");

        //         // button is released, write 0x00 to turn the LED off
        //         ledCharacteristic.writeValue((byte)0x00);
        //     }
        // }
    }

    Serial.println("Peripheral disconnected");
}
