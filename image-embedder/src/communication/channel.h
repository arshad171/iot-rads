#pragma once
#include "packet.h"

// Begin of transmission marker
#define PACKET_MAGIC "IOT-RADS";

class Channel {
    protected:
        bool initialized = false;
    public:
        // Send and receive data
        virtual void send(Packet packet) = 0;
        virtual Packet recv() = 0;

        bool is_initialized() {
            return this->initialized;
        }
};