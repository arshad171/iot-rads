#include <BasicLinearAlgebra.h>

#include "matrix_utils.h"

using namespace BLA;

// template<int r, int c>
// BLA::Matrix<r, 1> fun(BLA::Matrix<r, c> matrix1) {
//   BLA::Matrix<r, 1> ret;
//   ret.Fill(0.0);

//   for(int rr=0; rr < r; rr++) {
//     for(int cc=0; cc < c; cc++) {
//       ret(rr, 0) += matrix1(rr, cc);
//     }
//   }

//   return ret;
// }

template <int TinputSize, int ToutputSize, int TbatchSize>
class LinearLayer {
  public:
  boolean typeTrainable = true;

  // m, n, N
  int inputSize, outputSize, batchSize;

  BLA::Matrix<TinputSize, TbatchSize> lastX;
  BLA::Matrix<ToutputSize, TinputSize> weights;
  BLA::Matrix<ToutputSize, 1> bias;

  LinearLayer() {
    this->inputSize = TinputSize;
    this->outputSize = ToutputSize;
    this->batchSize = TbatchSize;

    lastX.Fill(0.0);
    weights.Fill(1.0);
    bias.Fill(1.0);
  }

  /*
  X: shape should be m x N
  persist: whether to keep track of last X
  return: Y: n x N
  */
  BLA::Matrix<ToutputSize, TbatchSize> forward(BLA::Matrix<TinputSize, TbatchSize> X) {
    BLA::Matrix<ToutputSize, TbatchSize> Y;
    
    // (n x m) x (m x 1) + (n x 1)
    Y = this->weights * X + this->bias;

    // update lastX
    this->lastX = X;
    
    return Y;
  }

  /*
  dLdY: upstream grads. shape should be n x N
  return: dLdX = m x N
  */
  BLA::Matrix<TinputSize, TbatchSize> backward(BLA::Matrix<ToutputSize, TbatchSize> dLdY) {
    BLA::Matrix<TinputSize, TbatchSize> dLdX;
    
    dLdX = this->weights * dLdY;
    
    return dLdX;
  }

  /*
  X: shape should be m x N
  dLdY: upstream grads. shape should be n x N
  return: dLdW: n x m, dLdb: n x 1
  */
  BLA::Matrix<ToutputSize, TinputSize> gradWeights(BLA::Matrix<ToutputSize, TbatchSize> dLdY) {
    BLA::Matrix<ToutputSize, TinputSize> dLdW;
    
    dLdW = dLdY * ~this.lastX;
    
    return dLdW;
  }

  /*
  X: shape should be m x N
  dLdY: upstream grads. shape should be n x N
  return: dLdW: n x m, dLdb: n x 1
  */
  BLA::Matrix<ToutputSize, 1> gradBias(BLA::Matrix<ToutputSize, TbatchSize> dLdY) {
    BLA::Matrix<ToutputSize, 1> dLdb;

    dLdb = sumCols(dLdY);
    
    return dLdb;
  }


};