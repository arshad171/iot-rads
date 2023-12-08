import os
import random
from pathlib import Path
from typing import Any, Tuple
from PIL import Image
import numpy as np
from scipy.io import loadmat
import cv2
import tensorflow as tf
import tensorflow_hub as hub


class ImageDataGenerator:
    def __init__(
        self, dataset_path, mask_path=None, use_dynamic_mask=True, image_size=[224, 224]
    ) -> None:
        self.MIN = -0.3
        self.MAX = 0.7
        self.use_dynamic_mask = use_dynamic_mask
        self.image_size = image_size
        self.path = Path(dataset_path)
        self.read_counter = 0

        if mask_path:
            mask = loadmat(mask_path)
            self.mask = mask["mask"]

        # self.embedding_layer = hub.KerasLayer(
        #     "https://www.kaggle.com/models/google/mobilenet-v3/frameworks/TensorFlow2/variations/small-075-224-feature-vector/versions/1",
        #     trainable=False,
        # )
        self.embedding_layer = tf.keras.Sequential(
            [
                tf.keras.layers.InputLayer(
                    input_shape=([image_size[0], image_size[1], 3])
                ),
                hub.KerasLayer(
                    "https://www.kaggle.com/models/google/mobilenet-v3/frameworks/TensorFlow2/variations/small-075-224-feature-vector/versions/1",
                    trainable=False,
                ),
                tf.keras.layers.Lambda(lambda x: tf.expand_dims(x, axis=2)),
                tf.keras.layers.AveragePooling1D(pool_size=8, padding="valid"),
            ]
        )

        images = os.listdir(self.path)
        self.images = list(filter(lambda filename: filename.endswith(".jpg"), images))
        self.length = len(self.images)

    def get_next_sample(self, raw=False) -> Tuple[np.ndarray, np.ndarray] | bytearray:
        """
        call this method get the next sample (x, y) pair.
        raw: True would return the raw bytes for transmission
        """
        (x, y) = self.__getitem__(self.read_counter)

        self.read_counter += 1

        if self.read_counter >= self.length:
            self.read_counter = 0
            random.shuffle(self.images)
            print("shuffling")

        if raw:
            return bytearray(x)
        else:
            return (x, y)

    def __len__(self):
        return self.length

    def read_image(self, path):
        # use PIL, cv2 reader throws redundant warnings
        image = Image.open(path)
        image = image.convert("RGB")
        image = np.array(image)

        return image

    def __getitem__(self, idx) -> Tuple[np.ndarray, np.ndarray]:
        image = self.read_image(os.path.join(self.path, self.images[idx]))

        # image = cv2.imread(os.path.join(self.path, self.images[idx]))

        image = self.transform(image, apply_mask=True)

        image_embedding = tf.squeeze(self.embedding_layer(image))
        image_embedding = (image_embedding - self.MIN) / (self.MAX - self.MIN)

        return image_embedding, image_embedding

    def __call__(self, *args: Any, **kwds: Any) -> Any:
        for idx in range(self.__len__()):
            yield self.__getitem__(idx)

    def get_image(self, idx, apply_mask=False):
        image = self.read_image(os.path.join(self.path, self.images[idx]))

        image = tf.squeeze(self.transform(image, apply_mask=apply_mask))

        return image

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
        A = np.array(
            [
                [-m1, 1],
                [-m2, 1],
            ]
        )

        b = np.array(
            [
                c1,
                c2,
            ]
        )

        point = np.linalg.solve(A, b)

        return (point[0], point[1])

    def get_mask(self, image):
        gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)

        # Apply edge detection (you may need to adjust parameters for your specific image)
        edges = cv2.Canny(gray, 50, 150, apertureSize=3)

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

        # mask out the right wall in the images by computing the intersection points

        # line1_m = -np.cos(horz_lines[min_horz][1]) / np.sin(horz_lines[min_horz][1])
        # line1_c = horz_lines[min_horz][0] / np.sin(horz_lines[min_horz][1])

        # line2_m = -np.cos(horz_lines[max_horz][1]) / np.sin(horz_lines[max_horz][1])
        # line2_c = horz_lines[max_horz][0] / np.sin(horz_lines[max_horz][1])

        # if vert_lines.size > 0:
        #     line3_m = -np.cos(vert_lines[0][1]) / np.sin(vert_lines[0][1])
        #     line3_c = vert_lines[0][0] / np.sin(vert_lines[0][1])

        #     i1 = self.find_intersection(line1_m, line3_m, line1_c, line3_c)
        #     i2 = self.find_intersection(line2_m, line3_m, line2_c, line3_c)
        #     if any(np.isnan(i1)) or any(np.isnan(i2)):
        #         poly_points = np.array([line1[0], line1[1], line2[1], line2[0]], dtype=np.int32)
        #     else:
        #         poly_points = np.array([line1[0], i1, i2, line2[0]], dtype=np.int32)
        # else:
        #     poly_points = np.array([line1[0], line1[1], line2[1], line2[0]], dtype=np.int32)

        # fill the mask
        poly_points = np.array([line1[0], line1[1], line2[1], line2[0]], dtype=np.int32)
        cv2.fillPoly(mask, [poly_points], (255, 255, 255))

        mask = np.array(mask > 0, dtype=np.float32)

        mask = cv2.resize(mask, dsize=self.image_size)

        return np.squeeze(mask)

    def transform(self, image, apply_mask=True):
        transformed_image = image

        # blur the image excluding the mask

        # blur_image = cv2.blur(transformed_image, ksize=(25, 25))
        # blur_image = cv2.resize(blur_image, dsize=self.image_size)

        transformed_image = cv2.resize(transformed_image, dsize=self.image_size)
        transformed_image = np.array(transformed_image, dtype=np.float32)

        if apply_mask:
            if self.use_dynamic_mask:
                mask = self.get_mask(image)
            else:
                mask = self.mask

            transformed_image *= mask

            # blur the image excluding the mask

            # transformed_image = (mask * transformed_image) + ((1 - mask) * blur_image)

        transformed_image = tf.expand_dims(transformed_image, axis=0)
        transformed_image = tf.cast(transformed_image, dtype=tf.float32)
        transformed_image /= 255

        return transformed_image
