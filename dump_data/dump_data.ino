#include <TinyMLShield.h>

byte IMAGE[320 * 240 * 2]; // QCIF: 176x144 x 2 bytes per pixel (RGB565)
int BYTES_PER_FRAME;
bool CAPTURE_FLAG;


void initializeCamera() {
    // cameraInit
    Serial.begin(9600);
    while (!Serial);
    initializeShield();

    // Initialize the OV7675 camera
    // if (!Camera.begin(QCIF, RGB565, 1, OV7675)) {
    if (!Camera.begin(QVGA, RGB565, 1, OV7675)) {
        Serial.println("Failed to initialize camera");
        while (1);
    }
    BYTES_PER_FRAME = Camera.width() * Camera.height() * Camera.bytesPerPixel();
}

void getImage() {
    Camera.readFrame(IMAGE);
    // delay(3000);
    Serial.println();
    Serial.println("BEGIN IMAGE");
    Serial.write(IMAGE, BYTES_PER_FRAME);
    // for (int i = 0; i < BYTES_PER_FRAME - 1; i += 2) {
    //     Serial.print("0x");
    //     Serial.print(IMAGE[i+1], HEX);
    //     Serial.print(IMAGE[i], HEX);
    //     if (i != BYTES_PER_FRAME - 2) {
    //         Serial.print(", ");
    //     }
    // }
    // Serial.println();
}

void setup() {
    // // Setup the board's status indicators
    // pinMode(LEDR,OUTPUT); pinMode(LEDG,OUTPUT); pinMode(LEDB,OUTPUT); pinMode(LED_PWR,OUTPUT);
    // digitalWrite(LEDR,1); digitalWrite(LEDG,1); digitalWrite(LEDB,1); digitalWrite(LED_PWR,0);

    // // Initialize serial port
    // SP.initialize(9600,100);

    // // Setup logging
    // set_log_lvl(LOG_LEVEL);
    // toggle_logs(true);

    // // Wait on the serial and notify user
    // digitalWrite(LEDB,0);
    // SP.blocking_wait(0);
    // digitalWrite(LEDB,1);

    // // Greet the user and present hardware info
    // LOG_SHORT(LOG_INFO,"Railway Anomaly Detection System v%s",FW_VERSION);

    // // Tell the user we completed setup
    // digitalWrite(LED_PWR,1);

    // camera init
    initializeCamera();
    // Do stuff
    // initialize();
}

void loop(){
    // String cameraCommand;
    // while (Serial.available()) {
    //   char c = Serial.read();
    //   if ((c != '\n') && (c != '\r')) {
    //     cameraCommand.concat(c);
    //   } 
    //   else if (c == '\r') {
    //     CAPTURE_FLAG = true;
    //     cameraCommand.toLowerCase();
    //   }
    // }
    // if (CAPTURE_FLAG && (cameraCommand == 'c')) {
    //   CAPTURE_FLAG = false;
    //   getImage();
       if (Serial.read() == 'c') {
         getImage();
      }

    // train();
}