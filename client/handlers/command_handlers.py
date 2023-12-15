""" Command handlers for the RADS protocol """
from PIL import Image,ImageShow
import numpy as np

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

    @handler.register_cmd(command=Command.SET_FEATURE_VECTOR)
    def handle_feature_vector(data: np.ndarray):
        print(data)
    
    @handler.register_cmd(command=Command.REPORT_ANOMALY)
    def handle_report(data: float):
        print(f">>> {data}")
