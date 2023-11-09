import numpy as np

from neural_network.layers.base_layer import BaseLayer
from neural_network.models.layer_types import LayerTypes

class SigmoidLayer(BaseLayer):
    type: LayerTypes = LayerTypes.ACTIVATION
    last_X: np.ndarray = None

    def forward(self, X: np.ndarray, persist: bool=False):
        """
        X: shape should be n x N
        persist: whether to keep track of last X
        return: Y: n x N
        """
        Y = 1 / (1 + np.exp(-X))
        # Y = np.exp(X) / (1 + np.exp(X))

        if persist:
            self.last_X = X

        return Y

    def backward(self, dLdY: np.ndarray):
        """
        X: shape should be n x N
        dLdY: upstream grads. shape should be n x N
        return: dLdX = n x N
        """
        X = self.last_X

        Y = self.forward(X)

        dYdX = Y * (1 - Y)
        dLdX = dLdY * dYdX

        return dLdX
