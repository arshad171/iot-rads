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
RichMatrix *BLAtoRichMatrix(BLA::Matrix<r,c,t> *matrix) {
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

size_t getRichMatrixSize(RichMatrix *matrix) {
    uint16_t r = matrix->metadata.rows;
    uint16_t c = matrix->metadata.cols;
    size_t s = matrix->metadata.element_size;

    return sizeof(MatrixMetadata) + r*c*s;
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

size_t getRichImageSize(RichImage *image) {
    uint16_t w = image->metadata.width;
    uint16_t h = image->metadata.height;
    byte c = image->metadata.channels;
    size_t d = image->metadata.depth;

    return w*h*c*d;
}