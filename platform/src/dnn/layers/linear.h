#pragma once
#include "../utils/matrix.h"

template<int TinputSize, int ToutputSize, int TbatchSize>
class LinearLayer {
public:
  boolean typeTrainable = true;

  // m, n, N
  int inputSize, outputSize, batchSize;

  BLA::Matrix<TinputSize, TbatchSize, float> lastX;
  BLA::Matrix<ToutputSize, TinputSize, float> weights;
  BLA::Matrix<ToutputSize, 1, float> bias;

  LinearLayer() {
    this->inputSize = TinputSize;
    this->outputSize = ToutputSize;
    this->batchSize = TbatchSize;

    this->lastX.Fill(0.0);
    this->bias.Fill(0.0);
    // init weights
    // unifrom initialzer
    for (int r = 0; r < ToutputSize; r++) {
      for (int c = 0; c < TinputSize; c++) {
        this->weights(r, c) = float(random(-1000, 1000)) / 1000.0;
      }
    }
  }

  /*
  X: shape should be m x N
  persist: whether to keep track of last X
  return: Y: n x N
  */
  BLA::Matrix<ToutputSize, TbatchSize> forward(BLA::Matrix<TinputSize, TbatchSize> X) {
    BLA::Matrix<ToutputSize, TbatchSize> Y;
    BLA::Matrix<ToutputSize, TbatchSize> biasBroadcast;
    biasBroadcast.Fill(0.0);

    for (int c = 0; c < TbatchSize; c++) {
      for (int r = 0; r < ToutputSize; r++) {
        biasBroadcast(r, c) = this->bias(r, 0);
      }
    }

    // (n x m) x (m x 1) + (n x 1)
    // (n x m) x (m x N) + (n x N)
    // Y = this->weights * X + this->bias;
    Y = this->weights * X + biasBroadcast;

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

    dLdX = ~this->weights * dLdY;

    return dLdX;
  }

  /*
  X: shape should be m x N
  dLdY: upstream grads. shape should be n x N
  return: dLdW: n x m, dLdb: n x 1
  */
  BLA::Matrix<ToutputSize, TinputSize> gradWeights(BLA::Matrix<ToutputSize, TbatchSize> dLdY) {
    BLA::Matrix<ToutputSize, TinputSize> dLdW;

    dLdW = dLdY * ~this->lastX;

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

  void copyWeightsToBuffer(int rowIndex, int colIndex, float *buffer, int size) {
    for (int c = 0; c < size; c++) {
      if (c >= TinputSize) {
        buffer[c] = 0.0;
      } else {
        buffer[c] = this->weights(rowIndex, c + colIndex);
      }
    }
  }

  void copyBiasToBuffer(int colIndex, float *buffer, int size) {
    for (int r = 0; r < size; r++) {
      if (r >= ToutputSize) {
        buffer[r] = 0.0;
      } else {
        buffer[r] = this->bias(r + colIndex, 0);
      }
    }
  }

  void copyWeightsFromBuffer(int rowIndex, int colIndex, float *buffer, int size) {
      for (int c = 0; (c < size) && (c + colIndex < TinputSize); c++) {
        this->weights(rowIndex, c + colIndex) = buffer[c];
      }
  }

  void copyBiasFromBuffer(int colIndex, float *buffer, int size) {
    for (int r = 0; (r < size) && (r + colIndex < ToutputSize); r++) {
      this->bias(r + colIndex, 0) = buffer[r];
    }
  }

  // void copyWeightsFromBufferAvg(int rowIndex, int colIndex, float *buffer, int size) {
  //   Serial.println("---");
  //     for (int c = 0; (c < size) && (c < TinputSize); c++) {
  //       // this->weights(rowIndex, c + colIndex) = 0.5 * (buffer[c] + this->weights(rowIndex, c + colIndex));
  //       float temp = this->weights(rowIndex, c + colIndex) + buffer[c];
  //       Serial.println(c);
  //       Serial.println(temp);
  //       this->weights(rowIndex, c + colIndex) = buffer[c];
  //     }
  // }

  // void copyBiasFromBufferAvg(int colIndex, float *buffer, int size) {
  //   for (int r = 0; (r < size) && (r < ToutputSize); r++) {
  //     this->bias(r + colIndex, 1) = 0.5 * (buffer[r] + this->bias(r + colIndex, 1));
  //   }
  // }
};