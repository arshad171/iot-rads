#include "../utils/common.h"
#include "types.h"
#include <BasicLinearAlgebra.h>
#include <stdlib.h>

template<int r, int c, typename t>
Matrix *BLAtoMatrix(BLA::Matrix<r,c,t> matrix) {
    size_t data_size = r * c * sizeof(t);
    Matrix *capsule = (Matrix *) memalloc(sizeof(MatrixMetadata) + data_size);

    // Fill in the metadata
    capsule->metadata.cols = c;
    capsule->metadata.rows = c;
    capsule->metadata.element_size = sizeof(t);

    // Copy the matrix data
    memcpy(capsule->data,matrix->storage,data_size);
    return capsule;
}

Image *initImage(uint16_t w, uint16_t h, byte c, size_t d) {
    Image *image = (Image *) memalloc(sizeof(ImageMetadata) + w*h*c*d);

    // Fill in the metadata
    image->metadata.channels = c;
    image->metadata.width = w;
    image->metadata.height = h;
    image->metadata.depth = d;

    return image;
}