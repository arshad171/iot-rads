""" RADS Project PC Client """

import time

from typing import Any
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
    return (data.decode("ascii"))

@handler.register_cmd(command=Command.WRITE_LOG)
def handle_log(data: Any):
    """ Just logs the string """
    print(data)

while True:
    handler.handle()
    time.sleep(INTERVAL_SECONDS)
