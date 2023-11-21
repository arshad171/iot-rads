#include <BasicLinearAlgebra.h>

#include "neural_network/layer_test.h"
#include "neural_network/base_layer.h"
#include "neural_network/linear_layer.h"
#include "neural_network/relu_layer.h"

using namespace BLA;

void setup() {
  // put your setup code here, to run once:
  // LayerTest* layer1 = new LayerTest();
  // layer1->function();

  // LayerTest layer2;
  // layer2.function();
  Serial.begin(9600);
}

void loop() {
  //test LinearLayer
  const int inputSize = 100;
  const int outputSize = 10;
  const int batchSize = 1;
  LinearLayer<inputSize, outputSize, batchSize> layer1;

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
  dLdX = layer1.backward(dLdY);
  dLdW = layer1.gradWeights(dLdY);
  dLdb = layer1.gradBias(dLdY);

  Serial.println(y(0, 0));
  Serial.println(y(9, 0));

  Serial.println(y.Rows);
  Serial.println(y.Cols);

  delay(200);


}
