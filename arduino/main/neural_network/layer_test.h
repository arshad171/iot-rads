#include <BasicLinearAlgebra.h>
// #include <Arduino_OV767X.h>

using namespace BLA;

class LayerTest{
  public:
    BLA::Matrix<10000, 10000, double> weights;
    BLA::Matrix<10000, 1, double> bias;

    LayerTest(){
      BLA::Matrix<100, 100, double> *m = new Matrix<100, 100, double>;
    }

    void function() {
      weights = weights * weights;
      bias = bias + bias; 
    }
};
