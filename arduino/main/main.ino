#include <BasicLinearAlgebra.h>

#include "neural_network/linear_layer.h"
#include "neural_network/relu_layer.h"
#include "neural_network/squared_loss.h"

using namespace BLA;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  //test LinearLayer
  // m, n, N
  const int inputSize = 100;
  const int outputSize = 10;
  const int batchSize = 1;
  LinearLayer<inputSize, outputSize, batchSize> layer1;
  ReLULayer<outputSize, batchSize> layer2;
  SquaredLoss<outputSize, batchSize> layer3;

  float loss;
  BLA::Matrix<inputSize, batchSize> x;
  BLA::Matrix<outputSize, batchSize> y;
  BLA::Matrix<outputSize, batchSize> dLdY;
  BLA::Matrix<inputSize, batchSize> dLdX;
  BLA::Matrix<outputSize, inputSize> dLdW;
  BLA::Matrix<outputSize, 1> dLdb;

  // assume inputs
  x.Fill(1.0);
  dLdY.Fill(0.5);

  y = layer1.forward(x);

  Serial.println("----- layer 1: linear out ------");
  Serial.println(y(0, 0));
  Serial.println(y(outputSize-1, 0));

  y = layer2.forward(y);

  Serial.println("----- layer 2: relu out ------");
  Serial.println(y(0, 0));
  Serial.println(y(outputSize-1, 0));

  loss = layer3.forward(y, y);

  Serial.println("----- layer 3: squared_loss out ------");
  Serial.println(loss);

  dLdX = layer1.backward(dLdY);
  dLdW = layer1.gradWeights(dLdY);
  dLdb = layer1.gradBias(dLdY);

  delay(200);


}
