import time
from protocol import SerialPort, DataType

from exceptions import (
    MalformedPacketException,
    InvalidCommandException,
    InvalidDataTypeException,
    SerialPortException
)

# How frequently to check for serial connection
CHECK_INTERVAL = 1

# Main loop of the program
print("Connecting to serial port...")

uphold_connection = True
while uphold_connection:
    try:
        with SerialPort("/dev/ttyACM0", 19200, 1) as port:
            do_read_packets = True
            while do_read_packets:
                packet = None

                try:
                    packet = port.read_packet()
                except MalformedPacketException as e:
                    print(f"**** RECEIVED MALFORMED PACKET: {e}")
                except InvalidCommandException as e:
                    print(f"**** RECEIVED INVALID COMMAND: {e}")
                except InvalidDataTypeException as e:
                    print(f"**** RECEIVED DATA OF INVALID TYPE: {e}")
                except SerialPortException:
                    print("Serial port connection interrupted. Restarting...")
                    do_read_packets = False

                # Print logs
                if packet is not None and packet.dtype == DataType.LOG:
                    print(packet.data.decode("ascii"))
    except SerialPortException:
        time.sleep(CHECK_INTERVAL)
