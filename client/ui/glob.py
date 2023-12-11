from PyQt6.QtCore import QObject,pyqtSignal
from PIL import Image

class __GlobalSignals(QObject):
    """ Holds global signal handlers for the UI """
    status_signal = pyqtSignal(str)
    flush_log_signal = pyqtSignal()
    flush_frame_signal = pyqtSignal

GLOBAL_SIGNALS = __GlobalSignals()
BLANK_IMAGE = Image.new("RGB",(320,240))
