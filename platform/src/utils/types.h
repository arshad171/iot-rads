#pragma once
#include "../utils/common.h"
#include <Arduino.h>
#include <stdlib.h>
#include <BasicLinearAlgebra.h>

enum MatrixType : uint16_t {
    TYPE_INT8,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_FLOAT32
};

struct MatrixMetadata {
    uint16_t rows;
    uint16_t cols;
    MatrixType type;
};

struct RichMatrix {
    MatrixMetadata metadata;
    byte data[];
};

template<int r, int c, typename t>
RichMatrix *BLAtoRichMatrix(BLA::Matrix<r,c,t> *matrix, MatrixType type) {
    size_t data_size = r * c * sizeof(t);
    RichMatrix *capsule = (RichMatrix *) memalloc(sizeof(MatrixMetadata) + data_size);

    // Fill in the metadata
    capsule->metadata.cols = c;
    capsule->metadata.rows = r;
    capsule->metadata.type = type;

    // Copy the matrix data
    memcpy(capsule->data,matrix->storage,data_size);
    return capsule;
}

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