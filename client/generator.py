""" Training/Testing data generator """

from abc import ABC,abstractmethod
from typing import Any, Tuple
from pathlib import Path
import math
import os
import random

import numpy as np
import tensorflow as tf
import tensorflow_hub as hub
import cv2
from sklearn.datasets import load_digits
from sklearn.model_selection import train_test_split
from scipy.io import loadmat
from PIL import Image


BATCH_SIZE = 2


class DataGenerator(ABC):
    # hold train and test data
    train_data: np.ndarray = None
    test_data: np.ndarray = None
    train_data_length: int = 0
    test_data_length: int = 0

    # counters to yield next batch
    train_read_ptr: int = 0
    test_read_ptr: int = 0

    @abstractmethod
    def load_data(self) -> None:
        """ load the dataset """
        raise NotImplementedError

    @abstractmethod
    def get_next_train_batch(self) -> np.ndarray:
        """ returns a numpy ndarray of shape (batch_size, num_feats) """
        raise NotImplementedError

    @abstractmethod
    def get_next_test_batch(self) -> np.ndarray:
        """ returns a numpy ndarray of shape (batch_size, num_feats) """
        raise NotImplementedError

    def length(self):
        """ returns a tuple of train and test data lengths """
        return (self.train_data_length, self.test_data_length)


class MNISTDataGenerator(DataGenerator):
    """ Data generator implementation for the MNIST digit dataset """
    def __init__(self) -> None:
        super().__init__()
        self.load_data()

    def load_data(self) -> None:
        digits = load_digits()

        x_train, x_test = train_test_split(digits.data, test_size=0.1)

        min_vals = np.min(x_train)
        max_vals = np.max(x_train)

        x_train = (x_train - min_vals) / (max_vals - min_vals)
        x_test = (x_test - min_vals) / (max_vals - min_vals)

        # drop the last batch
        x_train = x_train[: math.floor(x_train.shape[0] // BATCH_SIZE) * BATCH_SIZE]
        x_test = x_test[: math.floor(x_test.shape[0] // BATCH_SIZE) * BATCH_SIZE]

        # the data is already flat (-1, 64)
        self.train_data = x_train
        self.test_data = x_test

        self.train_data_length = x_train.shape[0]
        self.test_data_length = x_test.shape[0]

    def get_next_train_batch(self) -> np.ndarray:
        if self.train_read_ptr >= self.train_data_length:
            self.train_read_ptr = 0

        batch = self.train_data[self.train_read_ptr : self.train_read_ptr + BATCH_SIZE]
        self.train_read_ptr += BATCH_SIZE

        return batch

    def get_next_test_batch(self) -> np.ndarray:
        if self.test_read_ptr >= self.test_data_length:
            self.test_read_ptr = 0

        batch = self.test_data[self.test_read_ptr : self.test_read_ptr + BATCH_SIZE]
        self.test_read_ptr += BATCH_SIZE

        return batch



class ImageDataGenerator:
    """ Implementation of the generator using our image data"""
    def __init__(self,dataset_path,mask_path=None,use_dynamic_mask=True,image_size=[320, 240]) -> None:
        self.MIN = -0.3
        self.MAX = 0.7
        self.use_dynamic_mask = use_dynamic_mask
        self.image_size = image_size
        self.path = Path(dataset_path)
        self.read_counter = 0
        self.mask = None

        if mask_path:
            mask = loadmat(mask_path)
            self.mask = mask["mask"]
        self.use_mask = use_dynamic_mask or (self.mask is not None)

        self.embedding_network = tf.keras.models.load_model("../image-embedder/working.h5")

        images = os.listdir(self.path)
        self.images = list(filter(lambda filename: filename.endswith(".jpg"), images))
        self.length = len(self.images)

    # Automatic mask image generation
    def get_mask(self, image):
        # Apply edge detection (you may need to adjust parameters for your specific image)
        edges = cv2.Canny(image, 50, 150, apertureSize=3)

        # Use Hough Line Transform to detect lines
        lines = cv2.HoughLines(edges, 1, np.pi / 180, threshold=100)

        mask = np.zeros_like(edges)

        lines = np.squeeze(lines)

        theta_deg = lines[:, 1] * 180 / 3.14 + 90

        lines = np.hstack([lines, np.expand_dims(theta_deg, axis=1)])

        # filter horizontal lines
        horz_lines = lines[(lines[:, 2] > 170) & (lines[:, 2] < 190)]
        # filter by dist lines
        horz_lines = horz_lines[(horz_lines[:, 0] > 25) & (horz_lines[:, 0] < 225)]

        vert_lines = lines[(lines[:, 2] > 80) & (lines[:, 2] < 100)]
        vert_lines = vert_lines[(vert_lines[:, 0] > 100)]

        # closest and farthest horz lines
        min_horz = np.argmin(horz_lines[:, 0], axis=0)
        max_horz = np.argmax(horz_lines[:, 0], axis=0)

        line1 = self.get_line_points(horz_lines[min_horz][0], horz_lines[min_horz][1])
        line2 = self.get_line_points(horz_lines[max_horz][0], horz_lines[max_horz][1])

        # create a mask
        mask = np.zeros_like(image)

        poly_points = np.array([line1[0], line1[1], line2[1], line2[0]], dtype=np.int32)
        cv2.fillPoly(mask, [poly_points], (255, 255, 255))

        mask = np.array(mask > 0, dtype=np.float32)
        mask = cv2.resize(mask, dsize=self.image_size)

        return np.squeeze(mask)

    def get_line_points(self, rho, theta):
        # xy unit vectors
        a = np.cos(theta)
        b = np.sin(theta)
        # scale by rho
        x0 = a * rho
        y0 = b * rho

        x1 = int(x0 + 1000 * (-b))
        y1 = int(y0 + 1000 * (a))
        x2 = int(x0 - 1000 * (-b))
        y2 = int(y0 - 1000 * (a))

        return [(x1, y1), (x2, y2)]

    def find_intersection(self, m1, m2, c1, c2):
        A = np.array([
                [-m1, 1],
                [-m2, 1],
            ])
        b = np.array([
                c1,
                c2,
            ])

        point = np.linalg.solve(A, b)
        return (point[0], point[1])

    # Handles the Pythonic interface to the generator
    def __len__(self):
        return self.length

    def __call__(self, *args: Any, **kwds: Any) -> Any:
        for idx in range(self.__len__()):
            yield self.__getitem__(idx)

    def __getitem__(self, idx) -> Tuple[np.ndarray, np.ndarray]:
        image = self.read_image(os.path.join(self.path, self.images[idx]))
        transformed_image = self.transform(image).numpy()[0]
        embedding = self.get_feature_vector(image,apply_mask=self.use_mask)
        return embedding, transformed_image

    # Data extraction and manipulation
    def next(self, raw=False) -> Tuple[np.ndarray, np.ndarray] | bytearray:
        """
        call this method get the next sample (x, y) pair.
        raw: True would return the raw bytes for transmission
        """
        embedder,image = self.__getitem__(self.read_counter)

        self.read_counter += 1
        if self.read_counter >= self.length:
            self.read_counter = 0
            random.shuffle(self.images)
            print("shuffling")
        return bytearray(embedder) if raw else (embedder,image)

    def get_image(self, idx, apply_mask=True):
        """ Loads an indexed image from the dataset """
        image = self.read_image(os.path.join(self.path, self.images[idx]))
        image = tf.squeeze(self.transform(image, apply_mask=apply_mask))
        return image
    
    def read_image(self, path):
        """ Load an image from disk """
        image = Image.open(path)
        image = image.convert("L")
        image = np.array(image)
        return image

    def get_feature_vector(self, image, apply_mask=True, rescale=False):
        """ Extracts the feature vector of an image """
        transformed_image = self.transform(np.array(image),apply_mask)
        embedding = tf.squeeze(self.embedding_network(transformed_image))

        if rescale:
            embedding = (embedding - self.MIN) / (self.MAX - self.MIN)
        return embedding

    def transform(self, image, apply_mask=True):
        """ Transforms the image to a format suitable for the network """
        if isinstance(image, np.ndarray):
            transformed_image = image
        else:
            transformed_image = np.array(image)

        transformed_image = cv2.resize(transformed_image, dsize=self.image_size)
        transformed_image = np.array(transformed_image, dtype=np.float32)

        if apply_mask:
            if self.use_dynamic_mask:
                mask = self.get_mask(image)
            elif self.mask is not None:
                mask = self.mask
            else:
                print("Asked to apply mask but no mask provided")
            transformed_image *= mask

        transformed_image = tf.expand_dims(transformed_image, axis=0)
        transformed_image = tf.cast(transformed_image, dtype=tf.float32)
        transformed_image /= 255
        return transformed_image


class ImageDataGeneratorHandler(ImageDataGenerator):
    def __init__(self,dataset_path,mask_path=None,use_dynamic_mask=True,image_size=[320, 240]) -> None:
        super().__init__(dataset_path, mask_path, use_dynamic_mask, image_size)

    def next(self) -> Tuple[np.ndarray, np.ndarray] | bytearray:
        """ Get the next embedding vector and image """
        embedding,image = super().next(False)
        return np.expand_dims(embedding.numpy(),axis=1), image

    def get_feature_vector(self, image, apply_mask=True):
        return super().get_feature_vector(image, apply_mask)