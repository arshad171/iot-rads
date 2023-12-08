""" RADS Project PC Client """

import time
import struct
import numpy

from communicator.format import Command, DataType
from communicator.ports import SerialChannel
from communicator.protocol import Protocol

# General clock interval and pace of the program
INTERVAL_SECONDS = .1

# Define the protocol and command handlers
handler = Protocol(SerialChannel("/dev/ttyACM0",19200,INTERVAL_SECONDS))


@handler.register_type(dtype=DataType.LOG)
def decode_log(data: bytes):
    """ Decode the bytes into a string """
    return data.decode("ascii")


@handler.register_type(dtype=DataType.MAT)
def decode_mat(data: bytes):
    """ Transform the arduino matrix into a numpy matrix """
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
    return numpy.reshape(elements,(r,c))


@handler.register_cmd(command=Command.WRITE_LOG)
def handle_log(data: str):
    """ Just logs the string """
    print(data)

while True:
    handler.handle()
    time.sleep(INTERVAL_SECONDS)
