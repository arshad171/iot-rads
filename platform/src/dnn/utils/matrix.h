#pragma once
#include <BasicLinearAlgebra.h>

template<int r, int c, typename t = float>
float matrixDot(BLA::Matrix<r, c, t> matrix1, BLA::Matrix<r, c, t> matrix2) {
  float dot = 0.0;

  for(int rr=0; rr < r; rr++) {
    for(int cc=0; cc < c; cc++) {
      dot += matrix1(rr, cc) * matrix2(rr, cc);
    }
  }

  return dot;
}

template<int r, int c, typename t = float>
BLA::Matrix<r, c, t> elementMul(BLA::Matrix<r, c, t> matrix1, BLA::Matrix<r, c, t> matrix2) {
  BLA::Matrix<r, c, t> ret = 0.0;

  for(int rr=0; rr < r; rr++) {
    for(int cc=0; cc < c; cc++) {
      ret(rr, cc) = matrix1(rr, cc) * matrix2(rr, cc);
    }
  }

  return ret;
}

template<int r, int c, typename t = float>
BLA::Matrix<1, c, t> sumRows(BLA::Matrix<r, c, t> matrix) {
  BLA::Matrix<1, c, t> ret;
  ret.Fill(0.0);

  for(int cc=0; cc < c; cc++) {
    for(int rr=0; rr < r; rr++) {
      ret(0, cc) += matrix(rr, cc);
    }
  }

  return ret;
}

template<int r, int c, typename t = float>
BLA::Matrix<r, 1, t> sumCols(BLA::Matrix<r, c, t> matrix) {
  BLA::Matrix<r, 1, t> ret;
  ret.Fill(0.0);

  for(int rr=0; rr < r; rr++) {
    for(int cc=0; cc < c; cc++) {
      ret(rr, 0) += matrix(rr, cc);
    }
  }

  return ret;
}
