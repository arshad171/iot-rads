#include "../utils/common.h"
#include "camera.h"
#include <Arduino_OV767X.h>

static int FRAME_SZ;
static int COLORMODE;
static int BYTES_PER_PIXEL;
static bool INITIALIZED = false;

void camera_init(int res,int color,int fps) {
    LOG_SHORT(LOG_INFO,"Initializing camera module...");

    // Initialize the hardware module
    int retcode = Camera.begin(res,color,fps);
 
    if(!retcode) {
        LOG(LOG_FATAL,"Failed to initialize OV7675 camera module! (return code was %d)",retcode);
        die(RBOD::CAMERA_ERROR);
    } else {
        // DO NOT TRUST THE LIBRARY: It lies to us!
        if(color == GRAYSCALE) {
            BYTES_PER_PIXEL = 1;
        } else {
            BYTES_PER_PIXEL = 2;
        }

        FRAME_SZ = Camera.width()*Camera.height()*BYTES_PER_PIXEL;
        COLORMODE = color;

        LOG_SHORT(LOG_INFO,"Camera module initialized successfully");
        LOG_SHORT(LOG_INFO,"Camera resolution is %dx%dx%d",Camera.width(),Camera.height(),BYTES_PER_PIXEL);
        LOG_SHORT(LOG_INFO,"Size of one frame is %d bytes",FRAME_SZ);

        INITIALIZED = true;
    }
}

// ATTENTION! User of this pointer must free()!
RichImage *get_image() {
    if(!INITIALIZED) {
        LOG(LOG_ERROR,"Attempted to use the camera module without initialization!");
        return nullptr;
    }

    RichImage *image = initRichImage(Camera.width(), Camera.height(), COLORMODE, BYTES_PER_PIXEL);
    Camera.readFrame(image->data);

    return image;
}