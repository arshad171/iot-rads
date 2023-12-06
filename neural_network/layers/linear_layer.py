import numpy as np

from neural_network.layers.base_layer import BaseLayer
from neural_network.models.layer_types import LayerTypes


class LinearLayer(BaseLayer):
    name: str = "linear_layer"
    type: LayerTypes = LayerTypes.LINEAR
    last_X: np.ndarray = None

    # m
    input_size: int = None
    # n
    output_size: int = None
    # n x m
    weights: np.ndarray = None
    # n x 1
    bias: np.ndarray = None

    m_dLdW: np.ndarray = None
    m_dLdb: np.ndarray = None

    momentum: float = 0.0

    def __init__(
        self, input_size: int, output_size: int, name: str = "linear_layer"
    ) -> None:
        # initialize weights: Xavier normal
        std = np.sqrt(2 / (input_size + output_size))
        # self.weights = np.random.normal(
        #     loc=0.0, scale=std, size=(output_size, input_size)
        # )
        self.weights = np.random.uniform(low=-1.0, high=1.0, size=(output_size, input_size))
        self.bias = np.zeros(shape=(output_size, 1))

        self.m_dLdW = np.zeros_like(self.weights)
        self.m_dLdb = np.zeros_like(self.bias)

        self.input_size = input_size
        self.output_size = output_size
        self.name = name

    def forward(self, X: np.ndarray, persist=False) -> np.ndarray:
        """
        X: shape should be m x N
        persist: whether to keep track of last X
        return: Y: n x N
        """
        assert X.shape[0] == self.input_size, "forward: X shape mismatch"

        Y = self.weights @ X + self.bias

        Y = np.clip(Y, -10, 10)

        assert (
            Y.shape[0] == self.output_size and Y.shape[1] == X.shape[1]
        ), "forward: Y shape mismatch"

        if persist:
            self.last_X = X

        return Y

    def backward(self, dLdY: np.ndarray) -> np.ndarray:
        """
        dLdY: upstream grads. shape should be n x N
        return: dLdX = m x N
        """
        assert dLdY.shape[0] == self.output_size, "backwad: dLdY shape mismatch"

        dLdX = self.weights.T @ dLdY

        assert (
            dLdX.shape[0] == self.input_size and dLdX.shape[1] == dLdY.shape[1]
        ), "backwad: dLdX shape mismatch"

        return dLdX

    def grads(self, dLdY: np.ndarray) -> np.ndarray:
        """
        X: shape should be m x N
        dLdY: upstream grads. shape should be n x N
        return: dLdW: n x m, dLdb: n x 1
        """
        X = self.last_X

        assert X.shape[0] == self.input_size, "grads: X shape mismatch"
        assert dLdY.shape[0] == self.output_size, "grads: dLdY shape mismatch"

        dLdW = dLdY @ X.T

        dLdb = np.sum(dLdY, axis=1)
        dLdb = np.reshape(dLdb, newshape=(-1, 1))

        # momentum
        dLdW = self.momentum * self.m_dLdW + (1 - self.momentum) * dLdW
        dLdb = self.momentum * self.m_dLdb + (1 - self.momentum) * dLdb

        # update
        self.m_dLdW = dLdW
        self.m_dLdb = dLdb

        assert (
            dLdW.shape[0] == self.output_size and dLdW.shape[1] == self.input_size
        ), "grads: dLdX shape mismatch"
        assert (
            dLdb.shape[0] == self.output_size and dLdb.shape[1] == 1
        ), "grads: dLdb shape mismatch"

        return dLdW, dLdb
