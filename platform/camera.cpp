#include "camera.h"

#include <stdlib.h>
#include <Arduino_CRC32.h>

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

    // Enable the test pattern
    LOG_SHORT(LOG_DEBUG,"Enabling test pattern...");
    Camera.testPattern();

    // Read a frame
    LOG_SHORT(LOG_DEBUG,"Allocating framebuffer...");
    byte *data = (byte *) memalloc(frame_sz);

    LOG_SHORT(LOG_DEBUG,"Acquiring camera frame...");
    Camera.readFrame(data);
    LOG_SHORT(LOG_DEBUG,"Camera frame acquired");

    // Compute the checksum
    LOG_SHORT(LOG_DEBUG,"Computing frame CRC32...");

    Arduino_CRC32 crc32;
    if(crc32.calc(data,frame_sz) != 0x7D0D9B93) {
        LOG_SHORT(LOG_FATAL,"Camera module post-on self-test failed");
    }

    // Disable the test pattern
    LOG_SHORT(LOG_DEBUG,"Disabling test pattern...");
    Camera.noTestPattern();

    LOG_SHORT(LOG_INFO,"Camera module post-on self-test successful");
}

// ATTENTION! User of this pointer must free()!
byte *get_image() {
    byte *buffer = (byte *) memalloc(frame_sz*sizeof(byte));
    Camera.readFrame(buffer);

    return buffer;
}