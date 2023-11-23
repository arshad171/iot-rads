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

  /*
  X: shape should be m x N
  persist: whether to keep track of last X
  return: Y: n x N
  */
  BLA::Matrix<TinputSize, TbatchSize> forward(BLA::Matrix<TinputSize, TbatchSize> X) {
    BLA::Matrix<TinputSize, TbatchSize> Y;

    BLA::Matrix<TinputSize, TbatchSize, bool> temp = X > 0;

    Y = elementMul(X, temp.template Cast<float>());

    this->lastX = X;

    return Y;
  }

  /*
  dLdY: upstream grads. shape should be n x N
  return: dLdX = m x N
  */
  BLA::Matrix<TinputSize, TbatchSize> backward(BLA::Matrix<TinputSize, TbatchSize> dLdY) {
    BLA::Matrix<TinputSize, TbatchSize> dLdX, dYdX;

    BLA::Matrix<TinputSize, TbatchSize, bool> temp = this->lastX > 0;

    dYdX = temp.template Cast<float>();
    dLdX = elementMul(dLdY, dYdX);

    return dLdX;
  }

};