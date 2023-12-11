from PyQt6.QtCore import QObject,pyqtSignal
from PIL import Image

class GlobalSignals(QObject):
    status_signal = pyqtSignal(str)
    flush_log_signal = pyqtSignal()
    flush_frame_signal = pyqtSignal

GLOBAL_SIGNALS = GlobalSignals()
BLANK_IMAGE = Image.new("RGB",(320,240))
