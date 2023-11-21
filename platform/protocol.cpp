#include "protocol.h"
#include <string.h>

void send_data(byte *data,size_t sz,DType type,Cmd cmd,Channel *chan) {
    Packet packet;

    strcpy(packet.header.magic,packet_magic);
    packet.header.command = cmd;
    packet.header.size = sz;
    packet.header.type = type;
    packet.data = data;

    // Send over the channel
    chan->send(packet);
}

// Implement serial port packet handling
SerialPort::SerialPort(unsigned long baud,unsigned long timeout_ms) {
    Serial.begin(baud);
    Serial.setTimeout(timeout_ms);

    LOG_SHORT(LOG_INFO,"Initialized serial port with baud rate %ld and timeout %ld",baud,timeout_ms);
}

void SerialPort::send(Packet packet) {
    size_t hdr_sent_sz = Serial.write((byte *) &packet.header,sizeof(PHeader));
    size_t dat_sent_sz = Serial.write(packet.data,packet.header.size);

    if(hdr_sent_sz != sizeof(PHeader)) {
        LOG(LOG_ERROR,"Error sending header (sent %ld/%ld)",hdr_sent_sz,sizeof(PHeader));
    }

    if(dat_sent_sz != packet.header.size) {
        LOG(LOG_ERROR,"Error sending header (sent %ld/%ld)",dat_sent_sz,packet.header.size);
    }
}

// NOTE: User must free data when no longer necessary
// NOTE: Invalid packet is recognized by magic being all 0s
Packet SerialPort::recv() {
    // Initialize empty packet structure
    Packet packet;
    size_t rec_sz;

    size_t magic_length = sizeof(packet.header.magic);
    
    // Find a valid magic
    uint8_t valid_found = 0;
    while(valid_found < magic_length) {
        size_t read = Serial.readBytes(&packet.header.magic[valid_found],1);
        if(read != 1) {
            // We timed out - no data is available
            memset(&packet.header.magic,0,magic_length);
            return packet;
        }

        // Detect if the correct character has been read
        if(packet.header.magic[valid_found] == packet.header.magic[valid_found]) {
            valid_found++;
        } else {
            LOG(LOG_DEBUG,"Received malformed packet (invalid magic)");
            memset(&packet.header.magic,0,magic_length);
            return packet;
        }
    }

    // Valid magic has been received, read the rest
    rec_sz = Serial.readBytes((byte *) &packet.header+magic_length,sizeof(PHeader)-magic_length);
    if(rec_sz != sizeof(PHeader)-magic_length) {
        LOG(LOG_ERROR,"Received malformed packet (incomplete header)");
        return packet;
    }

    // Only allocate data if we *actually* have a payload
    if(packet.header.size > 0) {
        packet.data = (byte *) memalloc(packet.header.size);

        // Read the payload
        rec_sz = Serial.readBytes(packet.data,packet.header.size);
        if(rec_sz != packet.header.size) {
            LOG(LOG_ERROR,"Received malformed payload (expected %d bytes, received %d)",packet.header.size,rec_sz);
            free(packet.data);
            memset(&packet,0,sizeof(Packet));
            return packet;
        }
    }
}