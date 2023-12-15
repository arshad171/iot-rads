""" Command handlers for the RADS protocol """

import struct
from typing import Dict
import numpy as np
from PIL import Image

from ui.glob import GLOBAL_SIGNALS

from communicator.protocol import Protocol, DataType


def register_datatype_decoders(handler: Protocol):
    """Register decoders for the protocol"""

    @handler.register_decoder(dtype=DataType.LOG)
    def decode_log(data: bytes):
        """Decode the bytes into a string"""
        return data.decode("ascii")

    @handler.register_decoder(dtype=DataType.MAT)
    def decode_mat(data: bytes):
        """Decode the arduino matrix into a np matrix"""
        matrix_data_types = {
            0: "b",  # TYPE_INT8
            1: "B",  # TYPE_UINT8
            2: "h",  # TYPE_INT16
            3: "H",  # TYPE_UINT16
            4: "i",  # TYPE_INT32
            5: "I",  # TYPE_UINT32
            6: "f",  # TYPE_FLOAT32
        }

        if len(data) < 7:
            raise ValueError("Received invalid matrix data")

        r, c, t = struct.unpack("<HHH", data[:6])
        elements = struct.unpack(f"<{r*c}{matrix_data_types[t]}", data[6:])
        return np.reshape(elements, (r, c))

    @handler.register_decoder(dtype=DataType.IMG)
    def decode_img(data: bytes):
        """Decode the arduino image into a Pillow image"""
        image_cell_sizes = {1: "B", 2: "H", 4: "I"}  # Different sizes of cell
        image_cell_types = {0: "YUV422", 1: "RGB444", 2: "RGB565", 4: "GRAYSCALE"}

        if len(data) < 7:
            raise ValueError("Received invalid image data")

        w, h, f, d = struct.unpack("<HHBB", data[:6])
        GLOBAL_SIGNALS.status_signal.emit(f"Image cell size is {d}")
        elements = struct.unpack(f">{w*h}{image_cell_sizes[d]}", data[6:])

        # Handle color conversions
        if image_cell_types[f] == "RGB565":
            GLOBAL_SIGNALS.status_signal.emit("Received RGB565 Image")

            # Extract color information
            r = np.reshape([(e & 0xF800) >> 8 for e in elements], (h, w)).astype(
                np.uint8
            )
            g = np.reshape([(e & 0x07E0) >> 3 for e in elements], (h, w)).astype(
                np.uint8
            )
            b = np.reshape([(e & 0x001F) << 3 for e in elements], (h, w)).astype(
                np.uint8
            )

            # Convert to image
            return Image.fromarray(np.stack([r, g, b], axis=2))

        elif image_cell_types[f] == "GRAYSCALE":
            GLOBAL_SIGNALS.status_signal.emit("Received Grayscale Image")
            l = np.reshape(
                [e * (255 / (2 ** (d * 8))) for e in elements], (h, w)
            ).astype(np.uint8)
            return Image.fromarray(l)
        else:
            raise ValueError(f"Image format '{image_cell_types[f]}' not implemented.")

    @handler.register_decoder(dtype=DataType.WTS)
    def decode_layer_weights(data: bytes):
        # rows -> outputs
        # cols -> inputs
        rows, cols, layer_index = struct.unpack("<HHH", data[:6])

        weights = struct.unpack(f"<{rows*cols}f", data[6 : 6 + rows * cols * 4])
        bias = struct.unpack(f"<{rows}f", data[6 + rows * cols * 4 :])

        weights_mat = np.reshape(weights, newshape=(rows, cols), order="C")
        bias_mat = np.reshape(bias, newshape=(rows, 1), order="C")

        return layer_index, weights_mat, bias_mat

    @handler.register_decoder(dtype=DataType.FLT)
    def decode_float(data: bytes) -> float:
        value = struct.unpack("<f",data)
        return value[0]


def register_datatype_encoders(handler: Protocol):
    """Register encoders for the protocol"""

    @handler.register_encoder(dtype=DataType.LOG)
    def encode_log(data: str):
        """Encodes a string"""
        return data.encode("ascii")

    @handler.register_encoder(dtype=DataType.MAT)
    def encode_matrix(data: np.ndarray):
        """Encodes a numpy matrix"""
        if data.ndim != 2:
            raise ValueError(f"Unable to convert NP array with {data.ndim} dimensions")
        r, c = data.shape

        matrix_data_types = {
            "int8": ("b", 0),  # TYPE_INT8
            "uint8": ("B", 1),  # TYPE_UINT8
            "int16": ("h", 2),  # TYPE_INT16
            "uint16": ("H", 3),  # TYPE_UINT16
            "int32": ("i", 4),  # TYPE_INT32
            "uint32": ("I", 5),  # TYPE_UINT32
            "float32": ("f", 6),  # TYPE_FLOAT32
        }

        s_type, type_code = matrix_data_types[str(data.dtype)]
        fstr = f"<HHH{data.size}{s_type}"
        return struct.pack(fstr, r, c, type_code, *data.flatten(order="c"))

    @handler.register_encoder(dtype=DataType.WTS)
    def encode_weights(data: Dict):
        layer_index = data["layer_index"]
        rows, cols = data["weights"].shape

        print(layer_index, rows, cols)

        meta_bytes = struct.pack("<HHH", rows, cols, layer_index)
        weights_bytes = struct.pack(
            f"<{rows * cols}f", *data["weights"].flatten(order="C")
        )
        bias_bytes = struct.pack(f"<{rows}f", *data["bias"].flatten(order="C"))

        data_bytes = meta_bytes + weights_bytes + bias_bytes

        print(len(meta_bytes), len(weights_bytes), len(bias_bytes), len(data_bytes))

        return data_bytes
