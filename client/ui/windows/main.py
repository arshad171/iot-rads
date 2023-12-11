import os
import time
import typing

from threading import Event

from PIL import Image as PILImage
from PIL.Image import Image

from PyQt6 import QtGui, uic
from PyQt6.QtWidgets import QMainWindow,QPlainTextEdit,QStatusBar,QLabel,QFileDialog,QPushButton
from PyQt6.QtCore import QThread,pyqtSignal,pyqtSlot

from ui.glob import GLOBAL_SIGNALS

from communicator.format import Packet,Command,DataType
from communicator.protocol import Protocol
from communicator.ports import SerialChannel

import handlers.command_handlers as cmd_handlers
import handlers.datatype_handlers as type_handlers


class MainWindow(QMainWindow):
    """ Main window of the client """

    # Define command-related signals
    __signal_write_log = pyqtSignal(str)
    __signal_set_frame = pyqtSignal(Image)

    # Define internal signals
    signal_flush_log     = pyqtSignal()
    signal_status_update = pyqtSignal(str)

    def __init__(self,handler: Protocol):
        super().__init__() # Initialize the Qt interface
        uic.loadUi(os.path.join("ui","designer","main.ui"),self)
        self.show()
        self.setWindowTitle("RADS Client")

        # Protocol handler (for sending commands)
        self.__handler = handler

        # Load handles to relevant window components
        self.__log_txt = self.findChild(QPlainTextEdit,"log_txt")
        self.__status = self.findChild(QStatusBar,"statusbar")
        self.__frame = self.findChild(QLabel,"frame_lbl")
        self.__save_btn = self.findChild(QPushButton,"save_btn")
        self.__shutter = self.findChild(QPushButton,"shutter_btn")

        # Pictures received from Arduino
        self.__picture = None

        # Register command signal handlers
        self.__signal_write_log.connect(self.append_log)
        self.__signal_set_frame.connect(self.set_frame)

        # Register global signal handlers
        GLOBAL_SIGNALS.status_signal.connect(self.status_update)
        GLOBAL_SIGNALS.flush_log_signal.connect(self.flush_log)

        # Register UI signal handlers
        self.__save_btn.clicked.connect(self.save_picture)
        self.__shutter.clicked.connect(self.get_frame)

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
        print(status)
        self.__status.showMessage(status)

    @pyqtSlot(Image)
    def set_frame(self,image: Image):
        """ Sets the content of the image frame """
        self.__picture = image
        self.__frame.setPixmap(self.__picture.toqpixmap())

    @pyqtSlot()
    def save_picture(self):
        if self.__picture is None:
            return

        fname,_ = QFileDialog.getSaveFileName(self,"Save picture as...","","PNG Images (*.png)")
        if fname:
            self.__picture.save(fname,"PNG")

    @pyqtSlot()
    def get_frame(self):
        self.__handler.send(Packet(None,Command.GET_FRAME,DataType.CMD))


class MainThread(QThread):
    """ Main thread of the client """
    INTERVAL = .5

    def __init__(self):
        # Initialize PyQt interface
        super().__init__()

        # Setup the stop event
        self.__stop_event = Event()

        # Setup the protocol handler
        self.__handler = Protocol(SerialChannel("/dev/ttyACM0",19200,self.INTERVAL))
        cmd_handlers.register_command_handlers(self.__handler)
        type_handlers.register_datatype_handlers(self.__handler)

        # Spawn the window
        self.__window = MainWindow(self.__handler)

        # Register the QT signals with the protocol handler
        for cmd,sig in self.__window.command_signals.items():
            self.__handler.hook_cmd(cmd,sig)


    def run(self):
        """ Main loop of the client """

        while not self.__stop_event.is_set():
            self.__handler.handle()
            time.sleep(self.INTERVAL)

        self.__handler.stop()

    def stop(self):
        """ Stop the protocol handler """
        self.__stop_event.set()
