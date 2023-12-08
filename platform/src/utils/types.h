#pragma once
#include <Arduino.h>
#include <BasicLinearAlgebra.h>

struct MatrixMetadata {
    uint16_t rows;
    uint16_t cols;
    size_t element_size;
};

struct Matrix {
    MatrixMetadata metadata;
    byte data[];
};

template<int r, int c, typename t>
Matrix *BLAtoMatrix(BLA::Matrix<r,c,t> matrix);

struct ImageMetadata {
    uint16_t width;
    uint16_t height;
    byte channels;
    size_t depth;
};

struct Image {
    ImageMetadata metadata;
    byte data[];
};

Image *initImage(uint16_t w, uint16_t h, byte c, size_t d);
