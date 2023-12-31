#include <Arduino_OV767X.h>

// Device information (include experiment name)
#define FW_VERSION "0.0.1-Platform-Beta"

// Communication options
#define BAUD_RATE 19200
#define LOG_LEVEL LOG_DEBUG
#define PATIENCE 4

// Camera parameters
#define CAMERA_MODE  QVGA
#define CAMERA_COLOR GRAYSCALE
#define CAMERA_FPS   1

// BLE
#define BLE_ROLE PERIPHERAL

// Use random weights instead of acquired feature vectors
#define DNN_DEBUG false