#include "../utils/common.h"
#include "network.h"
#include "../utils/types.h"
#include "temp.h"
#include "../communication/protocol.h"

// Global variables
BLA::Matrix<FEATURE_DIM, BATCH_SIZE> xBatch;
NetworkShape network;

const float momentum = MOMENTUM;
const float learning_rate = LEARNING_RATE;
const int NUM_TRAINING_DATA = 100;
const int NUM_EPOCHS = 10;
const int NUM_ITERS = int(NUM_TRAINING_DATA / BATCH_SIZE);

// m, n, N
const int out1 = 10;

const int in2 = 10;
const int out2 = 2;

const int in3 = 2;
const int out3 = 10;

const int in4 = 10;
const int out4 = FEATURE_DIM;

SquaredLoss<out4, BATCH_SIZE> sq;

BLA::Matrix<out1, BATCH_SIZE> h1;
BLA::Matrix<out2, BATCH_SIZE> h2;
BLA::Matrix<out3, BATCH_SIZE> h3;
BLA::Matrix<out4, BATCH_SIZE> h4;

BLA::Matrix<out1, BATCH_SIZE> dLdY1;
BLA::Matrix<out2, BATCH_SIZE> dLdY2;
BLA::Matrix<out3, BATCH_SIZE> dLdY3;
BLA::Matrix<out4, BATCH_SIZE> dLdY4;

BLA::Matrix<FEATURE_DIM, BATCH_SIZE> dLdX1;
BLA::Matrix<in2, BATCH_SIZE> dLdX2;
BLA::Matrix<in3, BATCH_SIZE> dLdX3;
BLA::Matrix<in4, BATCH_SIZE> dLdX4;

BLA::Matrix<out1, FEATURE_DIM> dLdW1;
BLA::Matrix<out2, in2> dLdW2;
BLA::Matrix<out3, in3> dLdW3;
BLA::Matrix<out4, in4> dLdW4;

BLA::Matrix<out1, 1> dLdb1;
BLA::Matrix<out2, 1> dLdb2;
BLA::Matrix<out3, 1> dLdb3;
BLA::Matrix<out4, 1> dLdb4;



void updateXBatch(bool trainingData) {
  for (int batchIndex = 0; batchIndex < xBatch.Cols; batchIndex++) {
    bool success = false;
    while (!success) {
      request_feature_vector();
      success = receive_feature_vector(batchIndex);
    }
    LOG_SHORT(LOG_DEBUG, "got batch of size %d", BATCH_SIZE);
  }
}

float iterate(BLA::Matrix<FEATURE_DIM, BATCH_SIZE> x) {
  float loss;

  // forward
  h1 = network.lin1.forward(x);
  h1 = network.rel1.forward(h1);

  h2 = network.lin2.forward(h1);
  h2 = network.rel2.forward(h2);

  h3 = network.lin3.forward(h2);
  h3 = network.rel3.forward(h3);

  h4 = network.lin4.forward(h3);
  h4 = network.rel4.forward(h4);

  // loss
  loss = sq.forward(x, h4);
  dLdY4 = sq.backward(x, h4);

  // backward & gradient descent
  dLdY4 = network.rel4.backward(dLdY4);
  // dLdW4 = lin4.gradWeights(dLdY4);
  // dLdb4 = lin4.gradBias(dLdY4);
  dLdW4 *= momentum;
  dLdW4 += (network.lin4.gradWeights(dLdY4) * (1 - momentum));
  dLdb4 *= momentum;
  dLdb4 += (network.lin4.gradBias(dLdY4) * (1 - momentum));
  network.lin4.weights -= dLdW4 * learning_rate;
  network.lin4.bias -= dLdb4 * learning_rate;
  dLdY3 = network.lin4.backward(dLdY4);

  dLdY3 = network.rel3.backward(dLdY3);
  // dLdW3 = lin3.gradWeights(dLdY3);
  // dLdb3 = lin3.gradBias(dLdY3);
  dLdW3 *= momentum;
  dLdW3 += (network.lin3.gradWeights(dLdY3) * (1 - momentum));
  dLdb3 *= momentum;
  dLdb3 += (network.lin3.gradBias(dLdY3) * (1 - momentum));
  network.lin3.weights -= dLdW3 * learning_rate;
  network.lin3.bias -= dLdb3 * learning_rate;
  dLdY2 = network.lin3.backward(dLdY3);

  dLdY2 = network.rel2.backward(dLdY2);
  // dLdW2 = lin2.gradWeights(dLdY2);
  // dLdb2 = lin2.gradBias(dLdY2);
  dLdW2 *= momentum;
  dLdW2 += (network.lin2.gradWeights(dLdY2) * (1 - momentum));
  dLdb2 *= momentum;
  dLdb2 += (network.lin2.gradBias(dLdY2) * (1 - momentum));
  network.lin2.weights -= dLdW2 * learning_rate;
  network.lin2.bias -= dLdb2 * learning_rate;
  dLdY1 = network.lin2.backward(dLdY2);

  dLdY1 = network.rel1.backward(dLdY1);
  // dLdW1 = lin1.gradWeights(dLdY1);
  // dLdb1 = lin1.gradBias(dLdY1);
  dLdW1 *= momentum;
  dLdW1 += (network.lin1.gradWeights(dLdY1) * (1 - momentum));
  dLdb1 *= momentum;
  dLdb1 += (network.lin1.gradBias(dLdY1) * (1 - momentum));
  network.lin1.weights -= dLdW1 * learning_rate;
  network.lin1.bias -= dLdb1 * learning_rate;

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
