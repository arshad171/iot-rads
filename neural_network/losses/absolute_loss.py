import numpy as np


class AbsoluteLoss:
    def forward(self, Yhat: np.array, Y: np.array) -> np.ndarray:
        """
        Yhat, Y: shape should be n x N
        return L: the sfloat
        """

        assert (
            Yhat.shape[0] == Y.shape[0] and Yhat.shape[1] == Y.shape[1]
        ), "forward: Yhat, Y shape mismatch"

        Loss = np.linalg.norm(Yhat - Y, axis=0, ord=1)

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

        dLdY = ((Yhat - Y) > 0).astype(dtype=np.float32)

        return dLdY
