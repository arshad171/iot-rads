import numpy as np
from matplotlib.pyplot import *

from neural_network.layers.base_layer import BaseLayer
from neural_network.models.layer_types import LayerTypes


class ReluLayer(BaseLayer):
    name: str = None
    type: LayerTypes = LayerTypes.ACTIVATION
    # save the last X from forward pass for computing gradients
    last_X: np.ndarray = None

    def forward(self, X: np.ndarray, persist=False) -> np.ndarray:
        """
        X: shape should be m x N
        persist: whether to keep track of last X
        return: Y: n x N
        """
        Y = np.maximum(0, X)

        if persist:
            self.last_X = X

        return Y

    def backward(self, dLdY: np.ndarray) -> np.ndarray:
        """
        dLdY: upstream grads. shape should be n x N
        return: dLdX = m x N
        """
        X = self.last_X

        dYdX = X > 0
        dLdX = dLdY * dYdX

        return dLdX
