#include <BasicLinearAlgebra.h>

#include "matrix_utils.h"

using namespace BLA;

template <int TinputSize, int TbatchSize>
class ReLULayer {
  public:

  boolean typeTrainable = false;

  //m, n, N
  int inputSize, outputSize, batchSize;

  BLA::Matrix<TinputSize, TbatchSize> lastX;

  ReLULayer() {
    this->inputSize = TinputSize;
    this->outputSize = TinputSize;
    this->batchSize = TbatchSize;

    this->lastX.Fill(0.0);
  }

  

};