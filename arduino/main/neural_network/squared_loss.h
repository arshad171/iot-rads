#include "matrix_utils.h"

using namespace BLA;

template <int TinputSize, int TbatchSize>
class SquaredLoss {
  public:

  boolean typeTrainable = false;

  // m, n, N
  int inputSize, outputSize, batchSize;

  SquaredLoss() {
    this->inputSize = TinputSize;
    this-> outputSize = 1;
    this-> batchSize = TbatchSize;
  }

  /*
  Yhat, Y: shape should be n x N
  return L: the sfloat
  */
  float forward(BLA::Matrix<TinputSize, TbatchSize> Y, BLA::Matrix<TinputSize, TbatchSize> Yhat) {
    float loss = float(BLA::Norm(Yhat - Y) / float(TinputSize * TbatchSize));

    return loss;
  }

  /*
  Yhat, Y: shape should be n x N
  return dLdY: n x N
  */
  BLA::Matrix<TinputSize, TbatchSize> backward(BLA::Matrix<TinputSize, TbatchSize> Y, BLA::Matrix<TinputSize, TbatchSize> Yhat) {

    BLA::Matrix<TinputSize, TbatchSize> dLdY = Yhat - Y;

    return dLdY;
  }
};