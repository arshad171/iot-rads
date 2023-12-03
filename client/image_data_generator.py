import os
from pathlib import Path
from typing import Any
from scipy.io import loadmat
import cv2
import tensorflow as tf
import tensorflow_hub as hub


class ImageDataGenerator:
    def __init__(self, dataset_path, mask_path, image_size=[224, 224]) -> None:
        self.image_size = image_size
        self.path = Path(dataset_path)

        mask = loadmat(mask_path)
        self.mask = mask["mask"]

        self.embedding_layer = hub.KerasLayer(
            "https://www.kaggle.com/models/google/mobilenet-v3/frameworks/TensorFlow2/variations/small-075-224-feature-vector/versions/1",
            trainable=False,
        )

        images = os.listdir(self.path)
        self.images = list(filter(lambda filename: filename.endswith(".jpg"), images))
        self.length = len(self.images)

    def __len__(self):
        return self.length

    def __getitem__(self, idx):
        image = cv2.imread(os.path.join(self.path, self.images[idx]))
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        image = cv2.resize(image, dsize=self.image_size)

        image = tf.expand_dims(image, axis=0)
        image = tf.cast(image, dtype=tf.float32)
        image /= 255
        image *= self.mask

        image_embedding = tf.squeeze(self.embedding_layer(image))

        return image_embedding, image_embedding

    def __call__(self, *args: Any, **kwds: Any) -> Any:
        for idx in range(self.__len__()):
            yield self.__getitem__(idx)
