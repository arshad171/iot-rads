import numpy as np


class SquaredLoss:
    def forward(self, Yhat: np.array, Y: np.array) -> np.ndarray:
        """
        Yhat, Y: shape should be n x N
        return L: the sfloat
        """

        assert (
            Yhat.shape[0] == Y.shape[0] and Yhat.shape[1] == Y.shape[1]
        ), "forward: Yhat, Y shape mismatch"

        # Loss = np.linalg.norm(Yhat - Y, axis=0) ** 2
        Loss = np.square(Yhat - Y)

        Loss = np.mean(Loss)

        return Loss

    def backward(self, Yhat: np.array, Y: np.array) -> np.ndarray:
        """
        Yhat, Y: shape should be n x N
        return dLdY: n x N
        """

        assert (
            Yhat.shape[0] == Y.shape[0] and Yhat.shape[1] == Y.shape[1]
        ), "forward: Yhat, Y shape mismatch"

        dLdY = Yhat - Y

        return dLdY
