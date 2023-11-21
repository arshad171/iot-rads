#pragma once
#include "common.h"

// Begin of transmission marker
char *packet_magic = "IOT-RADS";

// Different kinds of data we can send
enum DType {
    CMD,
    LOG,
    TXT,
    DAT,
    IMG,
};

// Possible commands (that is, action requests) to be sent along packets
// RULES: All packets sent TO the arduino SHALL NOT have action "NONE"
enum Cmd {
    // Default (no specified action)
    NONE,

    // Track segmentation options
    GET_TRACK_MASK, // Used by the Arduino, send picture and request mask
    SET_TRACK_MASK, // Sent by the Cloud, set the track mask in memory

    // Anomaly detection
    REPORT_ANOMALY, // Used by the arduino to report an anomaly. Optionally an image may be attached
};

// Transmission packet
struct PHeader {
    char magic[8];
    Cmd command;
    size_t size;
    DType type;
};

struct Packet {
    PHeader header;
    byte *data;
};

// Transmission channel
class Channel {
    public:
        virtual void send(Packet packet) = 0;
        virtual Packet recv() = 0;
};

// Send data through some channel
void send_data(byte *data,size_t sz,DType type,Cmd cmd,Channel *chan)

// Serial port definition
class SerialPort : public Channel {
    public:
        SerialPort(unsigned long baud,unsigned long timeout_ms);
        void send(Packet packet);
        Packet recv();
};

