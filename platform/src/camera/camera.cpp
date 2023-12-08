#include "camera.h"

#include <stdlib.h>
#include <Arduino_OV767X.h>

static int frame_sz;

void camera_init(int res,int color,int fps) {
    LOG_SHORT(LOG_INFO,"Initializing camera module...");

    // Initialize the hardware module
    int retcode = Camera.begin(res,color,fps);
 
    if(!retcode) {
        LOG(LOG_FATAL,"Failed to initialize OV7675 camera module! (return code was %d)",retcode);
        die(RBOD::CAMERA_ERROR);
    } else {
        frame_sz = Camera.width()*Camera.height()*Camera.bytesPerPixel();

        LOG_SHORT(LOG_INFO,"Camera module initialized successfully");
        LOG_SHORT(LOG_INFO,"Camera resolution is %dx%dx%d",Camera.width(),Camera.height(),Camera.bitsPerPixel());
        LOG_SHORT(LOG_INFO,"Size of one frame is %d bytes",frame_sz);
    }
}

// ATTENTION! User of this pointer must free()!
byte *get_image() {
    byte *buffer = (byte *) memalloc(frame_sz*sizeof(byte));
    Camera.readFrame(buffer);

    return buffer;
}