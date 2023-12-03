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

        image = self.transform(image, apply_mask=True)

        image_embedding = tf.squeeze(self.embedding_layer(image))

        return image_embedding, image_embedding

    def __call__(self, *args: Any, **kwds: Any) -> Any:
        for idx in range(self.__len__()):
            yield self.__getitem__(idx)

    def get_image(self, idx, apply_mask=False):
        image = cv2.imread(os.path.join(self.path, self.images[idx]))

        image = tf.squeeze(self.transform(image, apply_mask=apply_mask))

        return image

    def transform(self, image, apply_mask=True):
        transformed_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        transformed_image = cv2.resize(transformed_image, dsize=self.image_size)

        transformed_image = tf.expand_dims(transformed_image, axis=0)
        transformed_image = tf.cast(transformed_image, dtype=tf.float32)
        transformed_image /= 255

        if apply_mask:
            transformed_image *= self.mask

        return transformed_image

        
