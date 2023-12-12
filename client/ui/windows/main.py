import os
import time
import typing

from threading import Event

import serial.tools.list_ports as lports

import numpy as np
from PIL import Image as PILImage
from PIL.Image import Image

from PyQt6 import QtGui, uic
from PyQt6.QtWidgets import QMainWindow,QPlainTextEdit,QStatusBar,QLabel,QFileDialog,QPushButton,QComboBox
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
    __signal_write_log1 = pyqtSignal(str)
    __signal_write_log2 = pyqtSignal(str)
    __signal_set_frame = pyqtSignal(Image)

    # Define internal signals
    signal_flush_log     = pyqtSignal()
    signal_status_update = pyqtSignal(str)

    def __init__(self,handler1: Protocol,handler2: Protocol,interval: int):
        super().__init__() # Initialize the Qt interface
        uic.loadUi(os.path.join("ui","designer","main.ui"),self)
        self.show()
        self.setWindowTitle("RADS Client")

        # Protocol handler (for sending commands)
        self.__interval = interval
        self.__handler1 = handler1
        self.__handler2 = handler2

        # Load handles to relevant window components
        self.__log1_txt = self.findChild(QPlainTextEdit,"log1_txt")
        self.__log2_txt = self.findChild(QPlainTextEdit,"log2_txt")
        self.__status = self.findChild(QStatusBar,"statusbar")
        self.__frame = self.findChild(QLabel,"frame_lbl")
        self.__save_btn = self.findChild(QPushButton,"save_btn")
        self.__shutter = self.findChild(QPushButton,"shutter_btn")
        self.__refresh = self.findChild(QPushButton,"refresh_btn")
        self.__ssel1 = self.findChild(QComboBox,"ssel1_cmb")
        self.__ssel2 = self.findChild(QComboBox,"ssel2_cmb")
        self.__conn1_btn = self.findChild(QPushButton,"conn1_btn")
        self.__conn2_btn = self.findChild(QPushButton,"conn2_btn")

        # List of attached arduino devices
        self.__devices = []

        # Pictures received from Arduino
        self.__picture = None

        # Register command signal handlers
        self.__signal_write_log1.connect(self.append_log1)
        self.__signal_write_log2.connect(self.append_log2)
        self.__signal_set_frame.connect(self.set_frame)

        # Register global signal handlers
        GLOBAL_SIGNALS.status_signal.connect(self.status_update)

        # Register UI signal handlers
        self.__save_btn.clicked.connect(self.save_picture)
        self.__shutter.clicked.connect(self.get_frame)
        self.__refresh.clicked.connect(self.refresh)
        self.__conn1_btn.clicked.connect(self.connection1)
        self.__conn2_btn.clicked.connect(self.connection2)

        # Create the signal dictionary
        self.command1_signals = {
            Command.WRITE_LOG: self.__signal_write_log1,
            Command.SET_FRAME: self.__signal_set_frame
        }

        self.command2_signals = {
            Command.WRITE_LOG: self.__signal_write_log2,
            Command.SET_FRAME: self.__signal_set_frame
        }

        # Initialize the comboboxes
        self.refresh()

    @pyqtSlot(str)
    def append_log1(self,log: str):
        """ Append log lines in the log pane """
        self.__log1_txt.appendPlainText(log)

    @pyqtSlot(str)
    def append_log2(self,log: str):
        """ Append log lines in the log pane """
        self.__log2_txt.appendPlainText(log)

    @pyqtSlot(str)
    def status_update(self,status: str):
        """ Update the status text """
        print(status)
        self.__status.showMessage(status)

    @pyqtSlot(Image)
    def set_frame(self,image: Image):
        """ Sets the content of the image frame """
        self.__picture = image
        self.__frame.clear()
        self.__frame.setPixmap(self.__picture.toqpixmap())
        self.__save_btn.setDisabled(False)

    @pyqtSlot()
    def save_picture(self):
        """ Saves picture to the device in PNG format """
        if self.__picture is None:
            return

        fname,_ = QFileDialog.getSaveFileName(self,"Save picture as...","","PNG Images (*.png)")
        if fname:
            self.__picture.save(fname,"PNG")

    @pyqtSlot()
    def get_frame(self):
        """ Sends a picture request to the arduino """
        self.__handler1.send(Packet(None,Command.GET_FRAME,DataType.CMD))

    @pyqtSlot()
    def refresh(self):
        """ Refresh the list of connected devices """
        self.__devices = [dev for dev in lports.comports() if dev.product == "Nano 33 BLE"]
        paths = [dev.device for dev in self.__devices]

        # Update the combo boxes
        self.__ssel1.clear()
        self.__ssel1.addItems(paths)
        self.__ssel2.clear()
        self.__ssel2.addItems(paths)

    @pyqtSlot()
    def connection1(self):
        """ Handles request to change the connection status of the first pane """
        if not self.__handler1.has_backend():
            self.__handler1.set_backend(SerialChannel(self.__ssel1.currentText(),19200,self.__interval))
            self.__conn1_btn.setText("D")
        else:
            self.__handler1.stop()
            self.__conn1_btn.setText("C")

    @pyqtSlot()
    def connection2(self):
        """ Handles request to change the connection status of the second pane """
        if not self.__handler2.has_backend():
            self.__handler2.set_backend(SerialChannel(self.__ssel2.currentText(),19200,self.__interval))
            self.__conn2_btn.setText("D")
        else:
            self.__handler2.stop()
            self.__conn2_btn.setText("C")


class MainThread(QThread):
    """ Main thread of the client """
    INTERVAL = .5

    def __init__(self):
        # Initialize PyQt interface
        super().__init__()

        # Setup the stop event
        self.__stop_event = Event()

        # Setup the protocol handler
        self.__handler1 = Protocol(None)
        cmd_handlers.register_command_handlers(self.__handler1)
        type_handlers.register_datatype_decoders(self.__handler1)
        type_handlers.register_datatype_encoders(self.__handler1)
        self.__handler2 = Protocol(None)
        cmd_handlers.register_command_handlers(self.__handler2)
        type_handlers.register_datatype_decoders(self.__handler2)
        type_handlers.register_datatype_encoders(self.__handler2)

        # Spawn the window
        self.__window = MainWindow(self.__handler1,self.__handler2,self.INTERVAL)

        # Register the QT signals with the protocol handler
        for cmd,sig in self.__window.command1_signals.items():
            self.__handler1.hook_cmd(cmd,sig)
        for cmd,sig in self.__window.command2_signals.items():
            self.__handler2.hook_cmd(cmd,sig)


    def run(self):
        """ Main loop of the client """

        while not self.__stop_event.is_set():
            if self.__handler1.has_backend():
                self.__handler1.handle()
            if self.__handler2.has_backend():
                self.__handler2.handle()
            time.sleep(self.INTERVAL)

        self.__handler1.stop()
        self.__handler2.stop()

    def stop(self):
        """ Stop the protocol handler """
        self.__stop_event.set()
