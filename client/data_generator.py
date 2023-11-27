import math
import numpy as np
from sklearn.datasets import load_digits
from sklearn.model_selection import train_test_split

BATCH_SIZE = 2


class DataGenerator:
    # hold train and test data
    train_data: np.ndarray = None
    test_data: np.ndarray = None
    train_data_length: int = 0
    test_data_length: int = 0

    # counters to yield next batch
    train_read_ptr: int = 0
    test_read_ptr: int = 0

    def load_data(self):
        raise NotImplementedError

    def get_next_train_batch(self):
        raise NotImplementedError

    def get_next_test_batch(self):
        raise NotImplementedError

    def length(self):
        return (self.train_data_length, self.test_data_length)


class MNISTDataGenerator(DataGenerator):
    def __init__(self) -> None:
        super().__init__()
        self.load_data()

    def load_data(self):
        digits = load_digits()

        x_train, x_test = train_test_split(digits.data, test_size=0.1)

        min_vals = np.min(x_train)
        max_vals = np.max(x_train)

        x_train = (x_train - min_vals) / (max_vals - min_vals)
        x_test = (x_test - min_vals) / (max_vals - min_vals)

        x_train = x_train[:math.floor(x_train.shape[0] // BATCH_SIZE) * BATCH_SIZE]
        x_test = x_test[:math.floor(x_test.shape[0] // BATCH_SIZE) * BATCH_SIZE]

        # the data is already flat (-1, 64)
        self.train_data = x_train
        self.test_data = x_test

        self.train_data_length = x_train.shape[0]
        self.test_data_length = x_test.shape[0]

    def get_next_train_batch(self):
        if self.train_read_ptr >= self.train_data_length:
            self.train_read_ptr = 0

        batch = self.train_data[self.train_read_ptr : self.train_read_ptr + BATCH_SIZE]

        self.train_read_ptr += BATCH_SIZE

        return batch

    def get_next_test_batch(self):
        if self.test_read_ptr >= self.test_data_length:
            self.test_read_ptr = 0

        batch = self.test_data[self.test_read_ptr : self.test_read_ptr + BATCH_SIZE]

        self.test_read_ptr += BATCH_SIZE

        return batch
