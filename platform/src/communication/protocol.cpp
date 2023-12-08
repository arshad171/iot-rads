#include "protocol.h"
#include <string.h>

// Begin of transmission marker
char *packet_magic = "IOT-RADS";

void pack(byte *data,size_t sz,DType type,Cmd cmd,Channel *chan) {
    Packet *packet = new Packet();

    strncpy(packet->header.magic,packet_magic,8);
    packet->header.command = cmd;
    packet->header.size = sz;
    packet->header.type = type;
    packet->data = data;

    // Send over the channel
    chan->send(*packet);

    delete packet;
}

