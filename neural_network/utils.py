from typing import List
import numpy as np

from neural_network.layers.base_layer import BaseLayer

def predict(neural_net: List[BaseLayer], X_data: np.ndarray, proba=True) -> np.ndarray:
    y_preds = []
    N = X_data.shape[0]
    
    for batch_start in range(0, N, 16):
        X_batch: np.ndarray = X_data[batch_start: batch_start + 16]
    
        X_batch = X_batch.T
    
        fp = X_batch
        for layer in neural_net:
            fp = layer.forward(fp, persist=False)
    
        if not proba:
            yp = np.array(fp.T > 0.5, dtype=np.float64)
        else:
            yp = np.array(fp.T, dtype=np.float64)

        y_preds.append(yp)
    
    y_preds = np.concatenate(y_preds, axis=0)

    return y_preds


def accuracy(neural_net: List[BaseLayer], X_data: np.ndarray, y_data: np.ndarray) -> np.float32:
    y_preds = predict(neural_net=neural_net, X_data=X_data, proba=False)
    y_preds = np.argmax(y_preds, axis=1)
    y_true = np.argmax(y_data, axis=1)
    
    return np.mean(y_preds == y_true)