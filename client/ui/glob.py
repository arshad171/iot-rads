from PyQt6.QtCore import QObject,pyqtSignal

class __GlobalSignals(QObject):
    """ Holds global signal handlers for the UI """
    status_signal = pyqtSignal(str)
    flush_log_signal = pyqtSignal()

GLOBAL_SIGNALS = __GlobalSignals()
