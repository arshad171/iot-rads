import random
from typing import Tuple

import numpy as np
from communicator.protocol import Protocol, Packet, Command, DataType
from image_data_generator import ImageDataGenerator


class ImageDataGeneratorHandler(ImageDataGenerator):
    def __init__(
        self,
        dataset_path,
        mask_path=None,
        use_dynamic_mask=True,
        image_size=[224, 224],
        downsample_factor=16,
    ) -> None:
        super().__init__(dataset_path, mask_path, use_dynamic_mask, image_size, downsample_factor)

        # self.handler: Protocol = handler

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

        return np.expand_dims(x.numpy(), axis=1)
