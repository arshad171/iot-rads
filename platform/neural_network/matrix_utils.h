#include <BasicLinearAlgebra.h>

using namespace BLA;

#ifndef _MATRIX_UTILS_H
#define _MATRIX_UTILS_H

// template<int r, int c>
// float matrixDot(BLA::Matrix<r, c> matrix1, BLA::Matrix<r, c> matrix2);

// template<int r, int c>
// BLA::Matrix<r, c> elementMul(BLA::Matrix<r, c> matrix1, BLA::Matrix<r, c> matrix2);

// template<int r, int c>
// BLA::Matrix<1, c> sumRows(BLA::Matrix<r, c> matrix);

// template<int r, int c>
// BLA::Matrix<r, 1> sumCols(BLA::Matrix<r, c> matrix);

template<int r, int c>
float matrixDot(BLA::Matrix<r, c> matrix1, BLA::Matrix<r, c> matrix2) {
  float dot = 0.0;

  for(int rr=0; rr < r; rr++) {
    for(int cc=0; cc < c; cc++) {
      dot += matrix1(rr, cc) * matrix2(rr, cc);
    }
  }

  return dot;
}

template<int r, int c>
BLA::Matrix<r, c> elementMul(BLA::Matrix<r, c> matrix1, BLA::Matrix<r, c> matrix2) {
  BLA::Matrix<r, c> ret = 0.0;

  for(int rr=0; rr < r; rr++) {
    for(int cc=0; cc < c; cc++) {
      ret(rr, cc) = matrix1(rr, cc) * matrix2(rr, cc);
    }
  }

  return ret;
}


template<int r, int c>
BLA::Matrix<1, c> sumRows(BLA::Matrix<r, c> matrix) {
  BLA::Matrix<1, c> ret;
  ret.Fill(0.0);

  for(int cc=0; cc < c; cc++) {
    for(int rr=0; rr < r; rr++) {
      ret(0, cc) += matrix(rr, cc);
    }
  }

  return ret;
}

template<int r, int c>
BLA::Matrix<r, 1> sumCols(BLA::Matrix<r, c> matrix) {
  BLA::Matrix<r, 1> ret;
  ret.Fill(0.0);

  for(int rr=0; rr < r; rr++) {
    for(int cc=0; cc < c; cc++) {
      ret(rr, 0) += matrix(rr, cc);
    }
  }

  return ret;
}

#endif