""" RADS Project PC Client """

import time

from communicator.format import Command
from communicator.ports import SerialChannel
from communicator.protocol import Protocol

# General clock interval and pace of the program
INTERVAL_SECONDS = .1

# Define the protocol and command handlers
handler = Protocol(SerialChannel("/dev/ttyACM0",19200,INTERVAL_SECONDS))

# Define a handler for log files
@handler.register_cmd(command=Command.NONE)
def handle_log(data: bytes):
    """ Just logs the string """
    print(data.decode("ascii"))

while True:
    handler.handle()
    time.sleep(INTERVAL_SECONDS)
