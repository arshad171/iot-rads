#pragma once
#include "../utils/common.h"

// ========================================== PROTOCOL STRUCTURE DEFINITION ===========================================
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

// =========================================== PROTOCOL CHANNEL DEFINITION ============================================
class Channel {
    protected:
        bool initialized = false;
    public:
        virtual void send(Packet packet) = 0;
        virtual Packet recv() = 0;

        bool is_initialized() {
            return this->initialized;
        }
};

// Serial port implementation
class SerialPort : public Channel {
    public:
        void initialize(unsigned long baud,unsigned long timeout_ms);
        void send(Packet packet);
        Packet recv();
        bool is_available();
        bool blocking_wait(unsigned long timeout_ms);
};

// =============================================== PROTOCOL ARDUINO API ===============================================
// Send data through some channel
void send_data(byte *data,size_t sz,DType type,Cmd cmd,Channel *chan);

// Make the global serial port object available
extern SerialPort SP;

