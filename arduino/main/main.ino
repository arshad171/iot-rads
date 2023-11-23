#include <BasicLinearAlgebra.h>

#include "neural_network/linear_layer.h"
#include "neural_network/relu_layer.h"
#include "neural_network/squared_loss.h"

using namespace BLA;

const float LEARNING_RATE = 0.0001;
const int BATCH_SIZE = 2;
const int NUM_TRAINING_DATA = 100;
const int NUM_EPOCHS = 10;
const int NUM_ITERS = int(NUM_TRAINING_DATA / BATCH_SIZE);

// m, n, N
const int in1 = 100;
const int out1 = 10;

const int in2 = 10;
const int out2 = 2;

const int in3 = 2;
const int out3 = 10;

const int in4 = 10;
const int out4 = 100;


LinearLayer<in1, out1, BATCH_SIZE> lin1;
ReLULayer<out1, BATCH_SIZE> rel1;

LinearLayer<in2, out2, BATCH_SIZE> lin2;
ReLULayer<out2, BATCH_SIZE> rel2;

LinearLayer<in3, out3, BATCH_SIZE> lin3;
ReLULayer<out3, BATCH_SIZE> rel3;

LinearLayer<in4, out4, BATCH_SIZE> lin4;
ReLULayer<out4, BATCH_SIZE> rel4;

SquaredLoss<out4, BATCH_SIZE> sq;

BLA::Matrix<in1, BATCH_SIZE> xBatch;

BLA::Matrix<out1, BATCH_SIZE> h1;
BLA::Matrix<out2, BATCH_SIZE> h2;
BLA::Matrix<out3, BATCH_SIZE> h3;
BLA::Matrix<out4, BATCH_SIZE> h4;

BLA::Matrix<out1, BATCH_SIZE> dLdY1;
BLA::Matrix<out2, BATCH_SIZE> dLdY2;
BLA::Matrix<out3, BATCH_SIZE> dLdY3;
BLA::Matrix<out4, BATCH_SIZE> dLdY4;

BLA::Matrix<in1, BATCH_SIZE> dLdX1;
BLA::Matrix<in2, BATCH_SIZE> dLdX2;
BLA::Matrix<in3, BATCH_SIZE> dLdX3;
BLA::Matrix<in4, BATCH_SIZE> dLdX4;

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
float iterate(BLA::Matrix<in1, BATCH_SIZE> x) {
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
  dLdW4 = lin4.gradWeights(dLdY4);
  dLdb4 = lin4.gradBias(dLdY4);
  lin4.weights -= dLdW4 * LEARNING_RATE;
  lin4.bias -= dLdb4 * LEARNING_RATE;
  dLdY3 = lin4.backward(dLdY4);

  dLdY3 = rel3.backward(dLdY3);
  dLdW3 = lin3.gradWeights(dLdY3);
  dLdb3 = lin3.gradBias(dLdY3);
  lin3.weights -= dLdW3 * LEARNING_RATE;
  lin3.bias -= dLdb3 * LEARNING_RATE;
  dLdY2 = lin3.backward(dLdY3);

  dLdY2 = rel2.backward(dLdY2);
  dLdW2 = lin2.gradWeights(dLdY2);
  dLdb2 = lin2.gradBias(dLdY2);
  lin2.weights -= dLdW2 * LEARNING_RATE;
  lin2.bias -= dLdb2 * LEARNING_RATE;
  dLdY1 = lin2.backward(dLdY2);

  dLdY1 = rel1.backward(dLdY1);
  dLdW1 = lin1.gradWeights(dLdY1);
  dLdb1 = lin1.gradBias(dLdY1);
  lin1.weights -= dLdW1 * LEARNING_RATE;
  lin1.bias -= dLdb1 * LEARNING_RATE;

  return loss;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while ( !Serial );

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

void loop() {
  delay(1000);

  float loss;
  // training
  for (int epoch = 0; epoch < NUM_EPOCHS; epoch++) {
    loss = 0.0;
    for (int iter = 0; iter < NUM_ITERS; iter++) {
      updateXBatch(true);
      loss += iterate(xBatch);
    }
    loss /= NUM_ITERS;
    Serial.print("epoch: "); Serial.print(epoch); Serial.print(" loss: "); Serial.print(loss); Serial.println();
  }


  // testing
  updateXBatch(true);
  loss = iterate(xBatch);
  Serial.print("train: "); Serial.print(loss); Serial.println();

  updateXBatch(false);
  loss = iterate(xBatch);

  Serial.print("test: "); Serial.print(loss); Serial.println();
}
