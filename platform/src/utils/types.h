#pragma once
#include <Arduino.h>
#include <BasicLinearAlgebra.h>
#include "common.h"

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

struct ImageMetadata {
    uint16_t width;
    uint16_t height;
    byte format;
    byte depth;
};

struct RichImage {
    ImageMetadata metadata;
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

    // Copy the matrix data (ROW MAJOR ORDER)
    memcpy(capsule->data,matrix->storage,data_size);
    return capsule;
}

size_t getRichMatrixSize(RichMatrix *matrix);
size_t getRichMatrixSize(uint16_t r, uint16_t c, MatrixType t);

RichImage *initRichImage(uint16_t w, uint16_t h, byte f, size_t d);

size_t getRichImageSize(RichImage *image);