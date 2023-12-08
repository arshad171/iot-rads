#pragma once
#include "../utils/common.h"
#include <Arduino.h>
#include <stdlib.h>
#include <BasicLinearAlgebra.h>

struct MatrixMetadata {
    uint16_t rows;
    uint16_t cols;
    size_t element_size;
};

struct RichMatrix {
    MatrixMetadata metadata;
    byte data[];
};

template<int r, int c, typename t>
RichMatrix *BLAtoMatrix(BLA::Matrix<r,c,t> *matrix) {
    size_t data_size = r * c * sizeof(t);
    RichMatrix *capsule = (RichMatrix *) memalloc(sizeof(MatrixMetadata) + data_size);

    // Fill in the metadata
    capsule->metadata.cols = c;
    capsule->metadata.rows = c;
    capsule->metadata.element_size = sizeof(t);

    // Copy the matrix data
    memcpy(capsule->data,matrix->storage,data_size);
    return capsule;
}

struct ImageMetadata {
    uint16_t width;
    uint16_t height;
    byte channels;
    size_t depth;
};

struct RichImage {
    ImageMetadata metadata;
    byte data[];
};

RichImage *initImage(uint16_t w, uint16_t h, byte c, size_t d) {
    RichImage *image = (RichImage *) memalloc(sizeof(ImageMetadata) + w*h*c*d);

    // Fill in the metadata
    image->metadata.channels = c;
    image->metadata.width = w;
    image->metadata.height = h;
    image->metadata.depth = d;

    return image;
}