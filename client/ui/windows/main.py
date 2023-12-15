import os
import time
from typing import List

from threading import Event

import serial.tools.list_ports as lports

import numpy as np
from PIL import Image as PILImage, ImageShow
from PIL.Image import Image

from PyQt6 import QtGui, uic
from PyQt6.QtWidgets import (
    QMainWindow,
    QPlainTextEdit,
    QStatusBar,
    QLabel,
    QFileDialog,
    QPushButton,
    QComboBox,
    QSpinBox,
    QMessageBox,
    QLineEdit
)
from PyQt6.QtCore import QThread, pyqtSignal, pyqtSlot
from generator import ImageDataGeneratorHandler

from ui.glob import GLOBAL_SIGNALS

from communicator.format import Packet, Command, DataType
from communicator.protocol import Protocol
from communicator.ports import SerialChannel

import handlers.command_handlers as cmd_handlers
import handlers.datatype_handlers as type_handlers


class MainWindow(QMainWindow):
    """Main window of the client"""
    # Define protocol-related signals
    # These must be statically declared
    __signal_append_log1 = pyqtSignal(str)
    __signal_append_log2 = pyqtSignal(str)
    __signal_append_log3 = pyqtSignal(str)
    __signal_picture_set = pyqtSignal(Image)
    __signal_b_roll_set  = pyqtSignal(Image)

    signal_message_box   = pyqtSignal(str,str,str)

    def __init__(self, handlers: List[Protocol], interval: int):
        super().__init__()  # Initialize the Qt interface
        uic.loadUi(os.path.join("ui", "designer", "main.ui"), self)
        self.show()
        self.setWindowTitle("RADS Client")

        # Load handles to relevant window components
        self.__log1_txt = self.findChild(QPlainTextEdit, "log1_txt")
        self.__log2_txt = self.findChild(QPlainTextEdit, "log2_txt")
        self.__log3_txt = self.findChild(QPlainTextEdit, "log3_txt")
        self.__status = self.findChild(QStatusBar, "statusbar")
        self.__frame = self.findChild(QLabel, "frame_lbl")
        self.__save_btn = self.findChild(QPushButton, "save_btn")
        self.__load_btn = self.findChild(QPushButton, "load_btn")
        self.__infer_btn = self.findChild(QPushButton,"infer_btn")
        self.__shutter = self.findChild(QPushButton, "shutter_btn")
        self.__refresh = self.findChild(QPushButton, "refresh_btn")
        self.__ssel1 = self.findChild(QComboBox, "ssel1_cmb")
        self.__ssel2 = self.findChild(QComboBox, "ssel2_cmb")
        self.__ssel3 = self.findChild(QComboBox, "ssel3_cmb")
        self.__conn1_btn = self.findChild(QPushButton, "conn1_btn")
        self.__conn2_btn = self.findChild(QPushButton, "conn2_btn")
        self.__conn3_btn = self.findChild(QPushButton, "conn3_btn")
        self.__pic_src_cmb = self.findChild(QComboBox,"src_pic_cmb")
        self.__data_src_cmb = self.findChild(QComboBox,"src_dat_cmb")
        self.__inference_src_cmb = self.findChild(QComboBox,"src_inf_cmb")
        self.__broll_shutter = self.findChild(QPushButton,"broll_btn")
        self.__broll_count = self.findChild(QSpinBox,"broll_sbox")
        self.__weight_src_cmb = self.findChild(QComboBox,"src_w_cmb")
        self.__weight_save_btn = self.findChild(QPushButton,"w_save_btn")
        self.__weight_load_btn = self.findChild(QPushButton,"w_load_btn")
        self.__thresh_text = self.findChild(QLineEdit,"thresh_txt")

        # List of attached arduino devices
        self.__devices = []

        # Pictures received from Arduino
        self.picture = None

        # Inference flag
        self.inference_requested = False

        # B-Rolls received from Arduibo
        self.__broll_path = None
        self.__broll_idx = None

        # Protocol handler (for sending commands)
        self.__interval = interval
        self.__handlers = handlers

        # Register protocol signal handlers
        self.__signal_append_log1.connect(self.append_log1)
        self.__signal_append_log2.connect(self.append_log2)
        self.__signal_append_log3.connect(self.append_log3)
        self.__signal_picture_set.connect(self.picture_set)
        self.__signal_b_roll_set.connect(self.handle_broll)

        # Register the message box handler
        self.signal_message_box.connect(self.display_alert)

        # Register global signal handlers
        GLOBAL_SIGNALS.status_signal.connect(self.update_statusbar)

        # Register UI signal handlers
        self.__save_btn.clicked.connect(self.save_picture)
        self.__load_btn.clicked.connect(self.load_picture)
        self.__shutter.clicked.connect(self.get_frame)
        self.__refresh.clicked.connect(self.refresh)
        self.__conn1_btn.clicked.connect(self.connection1)
        self.__conn2_btn.clicked.connect(self.connection2)
        self.__conn3_btn.clicked.connect(self.connection3)
        self.__broll_shutter.clicked.connect(self.begin_broll)
        self.__infer_btn.clicked.connect(self.infer)
        self.__weight_save_btn.clicked.connect(self.weight_save)
        self.__weight_load_btn.clicked.connect(self.weight_load)

        # Hook up signal handlers to the protocol
        self.__handlers[0].hook_cmd(Command.WRITE_LOG,self.__signal_append_log1)
        self.__handlers[0].hook_cmd(Command.SET_FRAME,self.__signal_picture_set)
        self.__handlers[1].hook_cmd(Command.WRITE_LOG,self.__signal_append_log2)
        self.__handlers[1].hook_cmd(Command.SET_FRAME,self.__signal_picture_set)
        self.__handlers[2].hook_cmd(Command.WRITE_LOG,self.__signal_append_log3)
        self.__handlers[2].hook_cmd(Command.SET_FRAME,self.__signal_picture_set)

        # Initialize the comboboxes
        self.refresh()

    def get_active_data_source(self) -> str:
        """ Returns the index of the active data source """
        return self.__data_src_cmb.currentText()

    def get_active_inference_source(self) -> int:
        """ Returns the index of the active data source """
        return self.__inference_src_cmb.currentIndex()

    def get_current_threshold(self) -> float:
        try:
            return float(self.__thresh_text.text())
        except ValueError:
            return 0.0

    # Signal handlers for UI-related actions
    @pyqtSlot(str)
    def append_log1(self, log: str):
        """Append log lines in the log pane"""
        self.__log1_txt.appendHtml(log)

    @pyqtSlot(str)
    def append_log2(self, log: str):
        """Append log lines in the log pane"""
        self.__log2_txt.appendHtml(log)

    @pyqtSlot(str)
    def append_log3(self, log: str):
        """Append log lines in the log pane"""
        self.__log3_txt.appendHtml(log)

    @pyqtSlot(str)
    def update_statusbar(self, status: str):
        """Update the status text"""
        print(status)
        self.__status.showMessage(status)

    @pyqtSlot(Image)
    def picture_set(self, image: Image):
        """Sets the content of the image frame"""
        self.picture = image
        self.__frame.clear()
        self.__frame.setPixmap(self.picture.toqpixmap())
        self.__save_btn.setDisabled(False)

    @pyqtSlot()
    def save_picture(self):
        """Saves picture to the device in PNG format"""
        if self.picture is None:
            return

        fname,_ = QFileDialog.getSaveFileName(self, "Save picture as...", "", "PNG Images (*.png)")
        if fname:
            self.picture.save(fname, "PNG")

    @pyqtSlot()
    def load_picture(self):
        """ Load picture from disk """
        fname,_ = QFileDialog.getOpenFileName(self,"Load picture...","","PNG Images (*.png);;JPEG Images (*.jpg)")
        if fname:
            self.picture_set(PILImage.open(fname))

    @pyqtSlot()
    def get_frame(self):
        """Sends a picture request to the arduino"""
        idx = self.__pic_src_cmb.currentIndex()
        self.__handlers[idx].send(Packet(None, Command.GET_FRAME, DataType.CMD))

    @pyqtSlot()
    def refresh(self):
        """Refresh the list of connected devices"""
        self.__devices = [
            dev for dev in lports.comports() if dev.product == "Nano 33 BLE"
        ]
        paths = [dev.device for dev in self.__devices]

        # Update the combo boxes
        self.__ssel1.clear()
        self.__ssel1.addItems(paths)
        self.__ssel2.clear()
        self.__ssel2.addItems(paths)
        self.__ssel3.clear()
        self.__ssel3.addItems(paths)

    @pyqtSlot()
    def connection1(self):
        """Handles request to change the connection status of the first pane"""
        if not self.__handlers[0].has_backend():
            self.__handlers[0].set_backend(SerialChannel(self.__ssel1.currentText(),19200,self.__interval))
            self.__log1_txt.clear()
            self.__conn1_btn.setText("D")
        else:
            self.__handlers[0].stop()
            self.__conn1_btn.setText("C")

    @pyqtSlot()
    def connection2(self):
        """Handles request to change the connection status of the second pane"""
        if not self.__handlers[1].has_backend():
            self.__handlers[1].set_backend(SerialChannel(self.__ssel2.currentText(),19200,self.__interval))
            self.__log2_txt.clear()
            self.__conn2_btn.setText("D")
        else:
            self.__handlers[1].stop()
            self.__conn2_btn.setText("C")

    @pyqtSlot()
    def connection3(self):
        """Handles request to change the connection status of the third pane"""
        if not self.__handlers[2].has_backend():
            self.__handlers[2].set_backend(SerialChannel(self.__ssel3.currentText(),19200,self.__interval))
            self.__log3_txt.clear()
            self.__conn3_btn.setText("D")
        else:
            self.__handlers[2].stop()
            self.__conn3_btn.setText("C")

    @pyqtSlot()
    def begin_broll(self):
        """ Begins a B-Roll procedure """
        self.__broll_path = QFileDialog.getExistingDirectory(self,"Select B-Roll directory")
        if not self.__broll_path:
            return
        self.__broll_idx = 1

        # Request B-Roll to the arduino
        src = self.__pic_src_cmb.currentIndex()
        self.__handlers[src].send(Packet(self.__broll_count.value().to_bytes(1,"little"),Command.GET_BROLL,DataType.DAT))

    @pyqtSlot(Image)
    def handle_broll(self,image: Image):
        """ Handles receiving B-Roll footage """
        self.picture_set(image)
        self.picture.save(os.path.join(self.__broll_path,f"{str(self.__broll_idx)}.png"), "PNG")
        self.__broll_idx += 1

    @pyqtSlot()
    def infer(self):
        """ Request inference """
        self.inference_requested = True

    @pyqtSlot()
    def weight_save(self):
        """ Request weights from device """
        self.__handlers[self.__weight_src_cmb.currentIndex()].send(Packet(None,Command.GET_WEIGHTS,DataType.CMD))
    
    @pyqtSlot()
    def weight_load(self):
        pass

    @pyqtSlot(str,str,str)
    def display_alert(self,title: str, text: str, icon: str):
        icons = {
            "info": QMessageBox.Icon.Information,
            "warn": QMessageBox.Icon.Warning,
            "err": QMessageBox.Icon.Critical
        }

        dlg = QMessageBox(self)
        dlg.setWindowTitle(title)
        dlg.setText(text)
        dlg.setIcon(icons[icon])
        dlg.exec()



class MainThread(QThread):
    """Main thread of the client"""
    INTERVAL = 0.1
    MIN = 7.175088256872186e-08
    MAX = 1.5143336895562243e-06

    def __init__(self):
        # Initialize PyQt interface
        super().__init__()

        # Setup the stop event
        self.__stop_event = Event()

        # Setup the protocol handler
        self.__curr_handler = -1
        self.__handlers = [
            Protocol(None),
            Protocol(None),
            Protocol(None)
        ]

        # Register handlers
        for handler in self.__handlers:
            cmd_handlers.register_command_handlers(handler)
            type_handlers.register_datatype_decoders(handler)
            type_handlers.register_datatype_encoders(handler)

            # Register internal command handlers
            handler.register_cmd_direct(
                Command.GET_FEATURE_VECTOR, self.__provide_feature_vector
            )
            handler.register_cmd_direct(
                Command.SET_FEATURE_VECTOR, self.__receive_feature_vector
            )
            # receive and save weights
            handler.register_cmd_direct(
                Command.SET_WEIGHTS, self.__save_weights
            )

            # receive inference results
            handler.register_cmd_direct(
                Command.REPORT_ANOMALY, self.__handle_inference_result
            )

        # Prepare the data generator
        self.__generator = ImageDataGeneratorHandler(
            "../images-data1/","../image-embedder/working.h5", use_dynamic_mask=True
        )

        # This will store the feature vector received by the embedder
        self.__feature_vector = None

        # Spawn the window
        self.__window = MainWindow(self.__handlers, self.INTERVAL)

    def run(self):
        """Main loop of the client"""

        while not self.__stop_event.is_set():
            for i,handler in enumerate(self.__handlers):
                self.__curr_handler = i
                if handler.has_backend():
                    handler.handle()
            time.sleep(self.INTERVAL)

        self.__handlers[0].stop()
        self.__handlers[1].stop()

    def stop(self):
        """Stop the protocol handler"""
        self.__stop_event.set()

    # Protocol handling
    def __receive_feature_vector(self, data: np.ndarray):
        self.__feature_vector = data.astype(np.float32)

    def __provide_feature_vector(self, data: None):
        data_src = self.__window.get_active_data_source()
        inference_src = self.__window.get_active_inference_source()
        if self.__window.inference_requested and self.__curr_handler == inference_src:
            ImageShow.show(self.__window.picture)
            features = self.__generator.get_feature_vector(self.__window.picture.convert("L"))
            print(features)

            GLOBAL_SIGNALS.status_signal.emit(f"Sending inference vector {str(features.shape).replace(' ','')}")
            self.__handlers[self.__curr_handler].send(Packet(features,Command.SET_INFERENCE_VECTOR,DataType.MAT))

            self.__window.inference_requested = False
        else:
            if data_src == "Arduino":
                # We get the feature vector from arduino
                if self.__feature_vector is not None:
                    GLOBAL_SIGNALS.status_signal.emit(f"Sending feature vector {str(self.__feature_vector.shape).replace(' ','')}")
                    self.__handlers[self.__curr_handler].send(Packet(self.__feature_vector,Command.SET_TRAINING_VECTOR,DataType.MAT))
                else:
                    self.__handlers[self.__curr_handler].send(Packet(None,Command.NO_FEATURE_VECTOR,DataType.CMD))
            elif data_src == "Client":
                # We use the internal data generator
                features,_ = self.__generator.next()
                GLOBAL_SIGNALS.status_signal.emit(f"Sending feature vector {str(features.shape).replace(' ','')}")
                self.__handlers[self.__curr_handler].send(Packet(features,Command.SET_TRAINING_VECTOR,DataType.MAT))
    
    def __handle_inference_result(self, data):
        thresh = self.__window.get_current_threshold()
        if data > thresh:
            self.__window.signal_message_box.emit("Anomaly detected!",f"The anomaly score of the picture is {data:.15f}.\n!!! ANOMALY DETECTED !!!","warn")
        else:
            self.__window.signal_message_box.emit("Inference results",f"The anomaly score of the picture is {data:.15f}","info")

    def __save_weights(self, data):
        layer_index, weights, bias = data

        np.save(f"{layer_index}_weights.npy", weights)
        np.save(f"{layer_index}_bias.npy", bias)
