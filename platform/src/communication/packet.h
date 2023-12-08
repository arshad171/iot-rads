#pragma once
#include <Arduino.h>

// Protocol structure definition
enum DType : byte { // Types of data that can be transmitted
    CMD,
    LOG,
    TXT,
    DAT,
    MAT,
    IMG
};

enum Cmd : byte { // Commands that can be sent to or received from other devices
    NONE, // No specified action (default)
    // Utility commands and intra-device coordination
    WRITE_LOG,
    REPORT_ANOMALY,
    // Training data manipulation
    GET_FEATURE_VECTOR, // Request feature vector from embedder
    SET_FEATURE_VECTOR, // Response to previous request
    // Image manipulation
    GET_FRAME, // Request frame from camera device
    SET_FRAME, // Response to previous request
};

// Protocol packet
struct PHeader {
    char magic[8] = { 0 };
    Cmd command;
    size_t size;
    DType type;
};

struct Packet {
    PHeader header;
    byte *data;
};
