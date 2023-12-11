""" Command handlers for the RADS protocol """
from PIL import Image,ImageShow

from communicator.protocol import Protocol,Command

def register_command_handlers(handler: Protocol):
    """ Register command handlers for the protocol """
    # @handler.register_cmd(command=Command.WRITE_LOG)
    # def handle_log(data: str):
    #     """ Just logs the string """
    #     print(data)

    # @handler.register_cmd(command=Command.SET_FRAME)
    # def handle_frame(data: Image):
    #     """ Save and show the image """
    #     ImageShow.show(data,"Test")
