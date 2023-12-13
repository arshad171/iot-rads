#pragma once
#include <Arduino.h>

// Protocol structure definition
enum DType : byte { // Types of data that can be transmitted
    LOG,
    MAT,
    IMG,
    CMD, // Empty packet - just command
    DAT
};

enum Cmd : byte { // Commands that can be sent to or received from other devices
    // Utility commands and intra-device coordination
    WRITE_LOG,
    REPORT_ANOMALY,
    // Training data manipulation
    GET_FEATURE_VECTOR, // Request feature vector from embedder
    SET_FEATURE_VECTOR, // Response to previous request
    // Image manipulation
    GET_FRAME, // Request frame from camera device
    SET_FRAME, // Response to previous request
    GET_BROLL,
    SET_BROLL
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
