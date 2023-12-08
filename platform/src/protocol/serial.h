#include "packet.h"

// Serial port wrapper for the RADS protocol
class SerialPort {
    private:
        bool initialized = false;
    public:
        void initialize(unsigned long baud,unsigned long timeout_ms);
        
        // Send and receive data
        void send(Packet packet);
        Packet recv();
        
        // Flow control
        bool is_available();
        bool is_initialized();
        bool blocking_wait(unsigned long timeout_ms);
};

// Global wrapper object
extern SerialPort SP;