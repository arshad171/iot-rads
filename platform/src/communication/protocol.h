#pragma once
#include <Arduino.h>
#include "channel.h"

void pack(byte *data,size_t sz,DType type,Cmd cmd,Channel *chan);