#include "../utils/common.h"
#include "../../settings/dnn.h"
#include "network.h"
#include <BasicLinearAlgebra.h>

// // NN Layers
#include "layers/linear.h"
#include "layers/relu.h"
#include "losses/squared.h"


const float momentum = MOMENTUM;
const float learning_rate = LEARNING_RATE;
const int batch_size = BATCH_SIZE;
const int NUM_TRAINING_DATA = 100;
const int NUM_EPOCHS = 10;
const int NUM_ITERS = int(NUM_TRAINING_DATA / batch_size);

// m, n, N
const int in1 = FEATURE_DIM;
const int out1 = 10;

const int in2 = 10;
const int out2 = 2;

const int in3 = 2;
const int out3 = 10;

const int in4 = 10;
const int out4 = FEATURE_DIM;


LinearLayer<in1, out1, batch_size> lin1;
ReLULayer<out1, batch_size> rel1;

LinearLayer<in2, out2, batch_size> lin2;
ReLULayer<out2, batch_size> rel2;

LinearLayer<in3, out3, batch_size> lin3;
ReLULayer<out3, batch_size> rel3;

LinearLayer<in4, out4, batch_size> lin4;
ReLULayer<out4, batch_size> rel4;

SquaredLoss<out4, batch_size> sq;

BLA::Matrix<in1, batch_size> xBatch;

BLA::Matrix<out1, batch_size> h1;
BLA::Matrix<out2, batch_size> h2;
BLA::Matrix<out3, batch_size> h3;
BLA::Matrix<out4, batch_size> h4;

BLA::Matrix<out1, batch_size> dLdY1;
BLA::Matrix<out2, batch_size> dLdY2;
BLA::Matrix<out3, batch_size> dLdY3;
BLA::Matrix<out4, batch_size> dLdY4;

BLA::Matrix<in1, batch_size> dLdX1;
BLA::Matrix<in2, batch_size> dLdX2;
BLA::Matrix<in3, batch_size> dLdX3;
BLA::Matrix<in4, batch_size> dLdX4;

BLA::Matrix<out1, in1> dLdW1;
BLA::Matrix<out2, in2> dLdW2;
BLA::Matrix<out3, in3> dLdW3;
BLA::Matrix<out4, in4> dLdW4;

BLA::Matrix<out1, 1> dLdb1;
BLA::Matrix<out2, 1> dLdb2;
BLA::Matrix<out3, 1> dLdb3;
BLA::Matrix<out4, 1> dLdb4;

void updateXBatch(bool trainingData) {
  for (int r = 0; r < xBatch.Rows; r++) {
    for (int c = 0; c < xBatch.Cols; c++) {
      if (trainingData) {
        // get training data here, request from serial
        xBatch(r, c) = float(random(-1000, 1000) / 1000.0);
      } else {
        // get test data here, request from serial
        xBatch(r, c) = float(random(-2000, 2000) / 1000.0);
      }
    }
  }
}

/*
do one forward and backward pass.
returns the reconsturction error
*/
float iterate(BLA::Matrix<in1, batch_size> x) {
  float loss;

  // forward
  h1 = lin1.forward(x);
  h1 = rel1.forward(h1);

  h2 = lin2.forward(h1);
  h2 = rel2.forward(h2);

  h3 = lin3.forward(h2);
  h3 = rel3.forward(h3);

  h4 = lin4.forward(h3);
  h4 = rel4.forward(h4);

  // loss
  loss = sq.forward(x, h4);
  dLdY4 = sq.backward(x, h4);

  // backward & gradient descent
  dLdY4 = rel4.backward(dLdY4);
  // dLdW4 = lin4.gradWeights(dLdY4);
  // dLdb4 = lin4.gradBias(dLdY4);
  dLdW4 *= momentum;
  dLdW4 += (lin4.gradWeights(dLdY4) * (1 - momentum));
  dLdb4 *= momentum;
  dLdb4 += (lin4.gradBias(dLdY4) * (1 - momentum));
  lin4.weights -= dLdW4 * learning_rate;
  lin4.bias -= dLdb4 * learning_rate;
  dLdY3 = lin4.backward(dLdY4);

  dLdY3 = rel3.backward(dLdY3);
  // dLdW3 = lin3.gradWeights(dLdY3);
  // dLdb3 = lin3.gradBias(dLdY3);
  dLdW3 *= momentum;
  dLdW3 += (lin3.gradWeights(dLdY3) * (1 - momentum));
  dLdb3 *= momentum;
  dLdb3 += (lin3.gradBias(dLdY3) * (1 - momentum));
  lin3.weights -= dLdW3 * learning_rate;
  lin3.bias -= dLdb3 * learning_rate;
  dLdY2 = lin3.backward(dLdY3);

  dLdY2 = rel2.backward(dLdY2);
  // dLdW2 = lin2.gradWeights(dLdY2);
  // dLdb2 = lin2.gradBias(dLdY2);
  dLdW2 *= momentum;
  dLdW2 += (lin2.gradWeights(dLdY2) * (1 - momentum));
  dLdb2 *= momentum;
  dLdb2 += (lin2.gradBias(dLdY2) * (1 - momentum));
  lin2.weights -= dLdW2 * learning_rate;
  lin2.bias -= dLdb2 * learning_rate;
  dLdY1 = lin2.backward(dLdY2);

  dLdY1 = rel1.backward(dLdY1);
  // dLdW1 = lin1.gradWeights(dLdY1);
  // dLdb1 = lin1.gradBias(dLdY1);
  dLdW1 *= momentum;
  dLdW1 += (lin1.gradWeights(dLdY1) * (1 - momentum));
  dLdb1 *= momentum;
  dLdb1 += (lin1.gradBias(dLdY1) * (1 - momentum));
  lin1.weights -= dLdW1 * learning_rate;
  lin1.bias -= dLdb1 * learning_rate;

   return loss;
 }

void initialize() {
  xBatch.Fill(0.0);

  h1.Fill(0.0);
  h2.Fill(0.0);
  h3.Fill(0.0);
  h4.Fill(0.0);

  dLdY1.Fill(0.0);
  dLdY2.Fill(0.0);
  dLdY3.Fill(0.0);
  dLdY4.Fill(0.0);

  dLdX1.Fill(0.0);
  dLdX2.Fill(0.0);
  dLdX3.Fill(0.0);
  dLdX4.Fill(0.0);

  dLdW1.Fill(0.0);
  dLdW2.Fill(0.0);
  dLdW3.Fill(0.0);
  dLdW4.Fill(0.0);

  dLdb1.Fill(0.0);
  dLdb2.Fill(0.0);
  dLdb3.Fill(0.0);
  dLdb4.Fill(0.0);
}

void train() {
  float loss;
  // training
  for (int epoch = 0; epoch < NUM_EPOCHS; epoch++) {
    loss = 0.0;
    for (int iter = 0; iter < NUM_ITERS; iter++) {
      updateXBatch(true);
      loss += iterate(xBatch);
    }
    loss /= NUM_ITERS;
    LOG_SHORT(LOG_INFO, "epoch: %d | loss: %f", epoch, loss);
  }


  // testing
  updateXBatch(true);
  loss = iterate(xBatch);
  LOG_SHORT(LOG_INFO, "train sample: %f", loss);

  updateXBatch(false);
  loss = iterate(xBatch);

  LOG_SHORT(LOG_INFO, "test sample: %f", loss);
}
