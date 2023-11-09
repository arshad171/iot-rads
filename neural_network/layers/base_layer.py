import numpy as np

from neural_network.models.layer_types import LayerTypes

class BaseLayer:
    name: str = None
    # type of layer (trainable/activation)
    type: LayerTypes = None
    # save the last X from forward pass for computing gradients
    last_X: np.ndarray = None

    def forward(self, X: np.ndarray, persist=False) -> np.ndarray:
        """
        X: shape should be m x N
        persist: whether to keep track of last X
        return: Y: n x N
        """
        pass

    def backward(self, dLdY: np.ndarray) -> np.ndarray:
        """
        dLdY: upstream grads. shape should be n x N
        return: dLdX = m x N
        """
        pass

    def grads(self, dLdY: np.ndarray) -> np.ndarray:
        """
        X: shape should be m x N
        dLdY: upstream grads. shape should be n x N
        return: dLdW: n x m, dLdb: n x 1
        """
        pass
