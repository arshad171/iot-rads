#pragma once
#include "../channel.h"

// Serial port wrapper for the RADS protocol
class SerialPort : public Channel {
    public:
        void initialize(unsigned long baud,unsigned long timeout_ms);
        
        // Send and receive data
        void send(Packet packet);
        Packet recv();
        
        // Flow control
        bool is_available();
        bool blocking_wait(unsigned long timeout_ms);
};