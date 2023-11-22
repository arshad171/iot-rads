#include <BasicLinearAlgebra.h>

#include "neural_network/linear_layer.h"
#include "neural_network/relu_layer.h"
#include "neural_network/squared_loss.h"

using namespace BLA;

float LEARNING_RATE = 0.0001;

// m, n, N
const int in1 = 100;
const int out1 = 10;

const int in2 = 10;
const int out2 = 2;

const int in3 = 2;
const int out3 = 10;

const int in4 = 10;
const int out4 = 100;

const int batchSize = 1;

LinearLayer<in1, out1, batchSize> lin1;
ReLULayer<out1, batchSize> rel1;

LinearLayer<in2, out2, batchSize> lin2;
ReLULayer<out2, batchSize> rel2;

LinearLayer<in3, out3, batchSize> lin3;
ReLULayer<out3, batchSize> rel3;

LinearLayer<in4, out4, batchSize> lin4;
ReLULayer<out4, batchSize> rel4;

SquaredLoss<out4, batchSize> sq;

float loss;
BLA::Matrix<in1, batchSize> x;

BLA::Matrix<out1, batchSize> h1;
BLA::Matrix<out2, batchSize> h2;
BLA::Matrix<out3, batchSize> h3;
BLA::Matrix<out4, batchSize> h4;

BLA::Matrix<out1, batchSize> dLdY1;
BLA::Matrix<out2, batchSize> dLdY2;
BLA::Matrix<out3, batchSize> dLdY3;
BLA::Matrix<out4, batchSize> dLdY4;

BLA::Matrix<in1, batchSize> dLdX1;
BLA::Matrix<in2, batchSize> dLdX2;
BLA::Matrix<in3, batchSize> dLdX3;
BLA::Matrix<in4, batchSize> dLdX4;

BLA::Matrix<out1, in1> dLdW1;
BLA::Matrix<out2, in2> dLdW2;
BLA::Matrix<out3, in3> dLdW3;
BLA::Matrix<out4, in4> dLdW4;

BLA::Matrix<out1, 1> dLdb1;
BLA::Matrix<out2, 1> dLdb2;
BLA::Matrix<out3, 1> dLdb3;
BLA::Matrix<out4, 1> dLdb4;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while ( !Serial );

  x.Fill(0.0);

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

  Serial.println("forward and backward done");
}

void loop() {
  Serial.println("hello");
  delay(1000);

}
