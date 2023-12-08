#include "../../utils/common.h"
#include "packet.h"
#include "serial.h"

void pack(byte *data,size_t sz,DType type,Cmd cmd,SerialPort *chan);