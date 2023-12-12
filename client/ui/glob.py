from PyQt6.QtCore import QObject,pyqtSignal

class __GlobalSignals(QObject):
    """ Holds global signal handlers for the UI """
    status_signal = pyqtSignal(str)

GLOBAL_SIGNALS = __GlobalSignals()
