import os
import time

from PIL import Image as PILImage
from PIL.Image import Image

from PyQt6 import uic, QtWidgets
from PyQt6.QtCore import QThread,pyqtSignal,pyqtSlot

from ui.glob import GLOBAL_SIGNALS,BLANK_IMAGE

from communicator.format import Command
from communicator.protocol import Protocol
from communicator.ports import SerialChannel

import handlers.command_handlers as cmd_handlers
import handlers.datatype_handlers as type_handlers


class MainWindow(QtWidgets.QMainWindow):
    """ Main window of the client """
    # Define command-related signals
    __signal_write_log = pyqtSignal(str)
    __signal_set_frame = pyqtSignal(Image)

    # Define internal signals
    signal_flush_log     = pyqtSignal()
    signal_status_update = pyqtSignal(str)

    def __init__(self):
        super().__init__() # Initialize the Qt interface
        uic.loadUi(os.path.join("ui","designer","main.ui"),self)
        self.show()

        # Load handles to relevant window components
        self.__log_txt = self.findChild(QtWidgets.QPlainTextEdit,"log_txt")
        self.__status = self.findChild(QtWidgets.QStatusBar,"statusbar")
        self.__frame = self.findChild(QtWidgets.QLabel,"frame_lbl")

        # Register command signal handlers
        self.__signal_write_log.connect(self.append_log)
        self.__signal_set_frame.connect(self.set_frame)

        # Register global signal handlers
        GLOBAL_SIGNALS.status_signal.connect(self.status_update)
        GLOBAL_SIGNALS.flush_log_signal.connect(self.flush_log)

        # Create the signal dictionary
        self.command_signals = {
            Command.WRITE_LOG: self.__signal_write_log,
            Command.SET_FRAME: self.__signal_set_frame
        }

    @pyqtSlot(str)
    def append_log(self,log: str):
        """ Append log lines in the log pane """
        self.__log_txt.appendPlainText(log)

    @pyqtSlot()
    def flush_log(self):
        """ Clear the log pane """
        self.__log_txt.clear()
    
    @pyqtSlot(str)
    def status_update(self,status: str):
        """ Update the status text """
        self.__status.showMessage(status)
    
    @pyqtSlot(Image)
    def set_frame(self,image: Image):
        self.__frame.setPixmap(image.toqpixmap())


class MainThread(QThread):
    """ Main thread of the client """
    INTERVAL = .5

    cctf = pyqtSignal(str)

    def __init__(self):
        # Initialize PyQt interface
        super().__init__()

        # Spawn the window
        self.__window = MainWindow()

        # Setup the protocol handler
        self.__handler = Protocol(SerialChannel("/dev/ttyACM0",19200,self.INTERVAL))
        cmd_handlers.register_command_handlers(self.__handler)
        type_handlers.register_datatype_handlers(self.__handler)

        # Register the QT signals with the protocol handler
        for cmd,sig in self.__window.command_signals.items():
            self.__handler.hook_cmd(cmd,sig)


    def run(self):
        """ Main loop of the client """
        while True:
            self.__handler.handle()
            time.sleep(self.INTERVAL)
