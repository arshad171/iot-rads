""" Command handlers for the RADS protocol """

import struct
import numpy as np
from PIL import Image

from ui.glob import GLOBAL_SIGNALS

from communicator.protocol import Protocol,DataType

def register_datatype_handlers(handler: Protocol):
    """ Register type handlers for the protocol """
    @handler.register_type(dtype=DataType.LOG)
    def decode_log(data: bytes):
        """ Decode the bytes into a string """
        return data.decode("ascii")


    @handler.register_type(dtype=DataType.MAT)
    def decode_mat(data: bytes):
        """ Transform the arduino matrix into a np matrix """
        matrix_data_types = {
            0: "b", # TYPE_INT8
            1: "B", # TYPE_UINT8
            2: "h", # TYPE_INT16
            3: "H", # TYPE_UINT16
            4: "i", # TYPE_INT32
            5: "I", # TYPE_UINT32
            6: "f"  # TYPE_FLOAT32
        }

        if len(data) < 7:
            raise ValueError("Received invalid matrix data")

        r,c,t = struct.unpack("<HHH",data[:6])
        elements = struct.unpack(f"<{r*c}{matrix_data_types[t]}",data[6:])
        return np.reshape(elements,(r,c))

    @handler.register_type(dtype=DataType.IMG)
    def decode_img(data: bytes):
        """ Transform the arduino image into a Pillow image """
        image_cell_sizes = {1: "B", 2: "H", 4: "I"} # Different sizes of cell
        image_cell_types = {0: "YUV422", 1: "RGB444", 2: "RGB565", 4: "GRAYSCALE"}

        if len(data) < 7:
            raise ValueError("Received invalid image data")

        w,h,f,d = struct.unpack("<HHBB",data[:6])
        GLOBAL_SIGNALS.status_signal.emit(f"Image cell size is {d}")
        elements = struct.unpack(f">{w*h}{image_cell_sizes[d]}",data[6:])

        # Handle color conversions
        if image_cell_types[f] == "RGB565":
            GLOBAL_SIGNALS.status_signal.emit("Received RGB565 Image")

            # Extract color information
            r = np.reshape([(e & 0xF800) >> 8 for e in elements],(h,w)).astype(np.uint8)
            g = np.reshape([(e & 0x07E0) >> 3 for e in elements],(h,w)).astype(np.uint8)
            b = np.reshape([(e & 0x001F) << 3 for e in elements],(h,w)).astype(np.uint8)

            # Convert to image
            return Image.fromarray(np.stack([r,g,b],axis=2))

        elif image_cell_types[f] == "GRAYSCALE":
            GLOBAL_SIGNALS.status_signal.emit("Received Grayscale Image")
            l = np.reshape([e*(255/(2**(d*8))) for e in elements],(h,w)).astype(np.uint8)
            return Image.fromarray(l)
        else:
            raise ValueError(f"Image format '{image_cell_types[f]}' not implemented.")
