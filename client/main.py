import time

from communicator.protocol import Packet, DataType
from communicator.ports import SerialPort
from exceptions import SerialPortException

# How frequently to check for serial connection
CHECK_INTERVAL = 1

print("Connecting to serial port...")
CONNECTED = False

# Keep connecting until we succeed
while not CONNECTED:
    try:
        port = SerialPort("/dev/ttyACM0",19200)
        CONNECTED = True
    except SerialPortException:
        time.sleep(1)

# Main loop of the program
while True:
    p : Packet = port.inbox.get()
    if p.dtype == DataType.LOG:
        print(p.data.decode("ascii"))
