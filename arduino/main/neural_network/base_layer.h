#include <BasicLinearAlgebra.h>

using namespace BLA;

template <int lastXR, int lastXC, typename lastXD = float>
class BaseLayer {
 public:
 boolean type = false;
 BLA::Matrix<lastXR, lastXC, lastXD> lastX;

 template <int fR, int fC, int xR, int xC, typename dt = float>
 BLA::Matrix<fR, fC> forward(BLA::Matrix<xR, xC> x);

 template <int bR, int bC, int dLdYR, int dLdYC, typename dt = float>
 BLA::Matrix<bR, bC> backward(BLA::Matrix<dLdYR, dLdYC> dLdY);

 template <int gR, int gC, int dLdYR, int dLdYC, typename dt = float>
 BLA::Matrix<gR, gC> grads(BLA::Matrix<dLdYR, dLdYC> dLdY);

};