#include "../../utils/common.h"
#include "serial.h"

static const char *packet_magic = PACKET_MAGIC;

void SerialPort::initialize(unsigned long baud,unsigned long timeout_ms) {
    Serial.begin(baud,SERIAL_8N1);
    Serial.setTimeout(timeout_ms);
    this->initialized = true;

    // We are initialized so this is safe
    LOG_SHORT(LOG_INFO,"Initialized serial port with baud rate %ld and timeout %ld",baud,timeout_ms);
}

void SerialPort::send(Packet packet) {
    // Do nothing if we are not initialized
    if(!this->initialized) {
        return; // No point in logging: the serial port is not available
    }

    size_t hdr_sent_sz = Serial.write((byte *) &packet.header,sizeof(PHeader));
    if(hdr_sent_sz != sizeof(PHeader)) {
        LOG(LOG_ERROR,"Error sending header (sent %ld/%ld)",hdr_sent_sz,sizeof(PHeader));
    }

    if(packet.data != nullptr) {
        size_t dat_sent_sz = Serial.write(packet.data,packet.header.size);
        if(dat_sent_sz != packet.header.size) {
            LOG(LOG_ERROR,"Error sending header (sent %ld/%ld)",dat_sent_sz,packet.header.size);
        }
    }
}

// NOTE: User must free data when no longer necessary
// NOTE: Invalid packet is recognized by magic being all 0s
Packet SerialPort::recv() {
    // Initialize empty packet structure
    Packet packet;
    size_t rec_sz;
    byte attempts;

    size_t magic_length = sizeof(packet.header.magic);

    // Safely initialize critical variables
    packet.data = nullptr;
    memset(packet.header.magic,0,magic_length);

    // Do nothing if we are not initialized
    if(!this->initialized) {
        return packet; // No point in logging: the serial port is not available
    }
    
    // Find a valid magic
    uint8_t valid_found = 0;
    while(valid_found < magic_length) {
        size_t read = Serial.readBytes(&packet.header.magic[valid_found],1);
        if(read != 1) {
            // We timed out - no data is available
            return packet;
        }

        // Detect if the correct character has been read
        if(packet.header.magic[valid_found] == packet_magic[valid_found]) {
            valid_found++;
        } else {
            LOG(LOG_DEBUG,"Received malformed packet (invalid magic)");
            memset(&packet.header.magic,0,magic_length);
            return packet;
        }
    }

    // Valid magic has been received, read the rest
    rec_sz = 0; attempts = 10;
    while(rec_sz < sizeof(PHeader)-magic_length && attempts > 0) {
        rec_sz += Serial.readBytes(((byte *) &(packet.header))+magic_length+rec_sz,sizeof(PHeader)-magic_length-rec_sz);
    }

    if(rec_sz != sizeof(PHeader)-magic_length) {
        LOG(LOG_ERROR,"Received malformed packet (incomplete header)");
        return packet;
    }

    // Only allocate data if we *actually* have a payload
    if(packet.header.size > 0) {
        packet.data = (byte *) memalloc(packet.header.size);

        // Read the payload
        rec_sz = 0; attempts = 10;
        while(rec_sz < packet.header.size && attempts > 0) {
            rec_sz += Serial.readBytes(packet.data+rec_sz,packet.header.size-rec_sz);
            attempts--;
        }

        if(rec_sz != packet.header.size) {
            LOG(LOG_ERROR,"Received malformed payload (expected %d bytes, received %d)",packet.header.size,rec_sz);
            free(packet.data);
            memset(&packet,0,sizeof(Packet));
            return packet;
        }
    }
}

// Can we send data to the serial port?
bool SerialPort::is_available() {
    if(Serial) return true; else return false;
}

// Wait until the serial port becomes available (set timeout to 0 for indefinite wait)
// Return value is whether SP is accessible after waiting for the specified time
bool SerialPort::blocking_wait(unsigned long timeout_ms) {
    unsigned long limit_time = millis() + timeout_ms;
    while(!this->is_available() && (timeout_ms == 0 || millis() <= limit_time));
    return this->is_available();
}

// Global serial port wrapper object
SerialPort SP;