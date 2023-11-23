from protocol import SerialPort,Packet,DataType
from exceptions import *

with SerialPort("/dev/ttyACM0",9600,1) as port:
    while True:
        try:
            packet = port.read_packet()

            # Print logs
            if packet is not None and packet.dtype == DataType.LOG:
                print(packet.data.decode("ascii"))

        except MalformedPacketException as e:
            pass # Just ignore them (for now)