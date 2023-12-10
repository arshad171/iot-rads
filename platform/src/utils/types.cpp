#include "../utils/common.h"
#include "types.h"

size_t getRichMatrixSize(RichMatrix *matrix) {
    uint16_t r = matrix->metadata.rows;
    uint16_t c = matrix->metadata.cols;

    // Retrieve element size
    size_t element_size;
    switch(matrix->metadata.type) {
        case MatrixType::TYPE_INT8:
        case MatrixType::TYPE_UINT8:
            element_size = 1;
            break;
        case MatrixType::TYPE_INT16:
        case MatrixType::TYPE_UINT16:
            element_size = 2;
            break;
        case MatrixType::TYPE_INT32:
        case MatrixType::TYPE_UINT32:
        case MatrixType::TYPE_FLOAT32:
            element_size = 4;
            break;
        default:
            // If we get here we're screwed but we try to salvage it
            LOG(LOG_ERROR,"Invalid matrix data type specified!");
            element_size = 4;
    }

    return sizeof(MatrixMetadata) + r*c*element_size;
}

RichImage *initRichImage(uint16_t w, uint16_t h, byte f, size_t d) {
    RichImage *image = (RichImage *) memalloc(sizeof(ImageMetadata) + w*h*d);

    // Fill in the metadata
    image->metadata.format = (byte) f;
    image->metadata.width = w;
    image->metadata.height = h;
    image->metadata.depth = d;

    return image;
}

size_t getRichImageSize(RichImage *image) {
    uint16_t w = image->metadata.width;
    uint16_t h = image->metadata.height;
    size_t d = image->metadata.depth;

    return sizeof(ImageMetadata) + w*h*d;
}
