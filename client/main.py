""" RADS Project PC Client """
import sys
from PyQt6 import QtWidgets as QT

from ui.windows.main import MainThread
from ui.glob import GLOBAL_SIGNALS

if __name__ == "__main__":
    app = QT.QApplication(sys.argv)
    main_window = MainThread()

    main_window.start()
    app.exec()
