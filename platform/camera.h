#pragma once
#include "common.h"

#include <Arduino_OV767X.h>

void camera_init(int,int,int);
byte *get_image();
