""" Communication ports for the RADS network """

from queue import SimpleQueue
from threading import Thread
import struct

from serial import Serial,SerialException,EIGHTBITS,STOPBITS_ONE,PARITY_NONE
from exceptions import SerialPortException,MalformedPacketException
from communicator.protocol import Packet,DataType,Command


class SerialPort(Thread):
    """" Implements a serial port (together with the RADS protocol) """
    def __init__(self, port: str = None, baud: int = 9600):
        # Initialize the two letterboxes
        self.inbox = SimpleQueue()
        self.outbox = SimpleQueue()

        # Setup the serial connection
        try:
            self.serial = Serial(port,baud,timeout=0.1,parity=PARITY_NONE,
                                 bytesize=EIGHTBITS,stopbits=STOPBITS_ONE)
            self.open = True
        except SerialException as e:
            raise SerialPortException("Unable to open serial port") from e

        # Initiate thread
        super().__init__(name="Serial Port Connection")
        self.start()

    def run(self):
        if not self.open:
            raise SerialPortException("Serial port state invalid.")

        while True:
            # Check whether we have packets to send
            while not self.outbox.empty():
                self.send_packet(self.outbox.get(False))

            # Process any incoming packet
            if (p := self.read_packet()) is not None:
                self.inbox.put(p)

    def send_packet(self, packet: Packet):
        """ Send a packet over the serial port """
        try:
            self.serial.write(packet.serialize())
        except SerialException as e:
            raise SerialPortException("Unable to send data through the serial port") from e

    def read_packet(self) -> Packet:
        """ Receive a packet from the serial port (if any) """
        try:
            # Read the stream until we find a MAGIC
            lead = self.serial.read_until(expected=Packet.magic)

            # Validate that we actually got a good MAGIC
            if len(lead) == 0:
                return None
            if not lead.endswith(Packet.magic):
                raise MalformedPacketException("Invalid MAGIC received")

            header_format = "<III"  # (uint,size_t,uint)
            header_data = self.serial.read(struct.calcsize(header_format))

            if len(header_data) != struct.calcsize(header_format):
                raise MalformedPacketException("Invalid header received")

            # Get the header fields
            cmd_id, data_size, type_id = struct.unpack(header_format, header_data)

            # Now we can read the data
            payload = self.serial.read(data_size)
            if len(payload) != data_size:
                raise MalformedPacketException("Incomplete data stream")

            return Packet(payload, Command.by_id(cmd_id), DataType.by_id(type_id))
        except SerialException as e:
            raise SerialPortException(
                "An error occurred while trying to read data from the serial port"
            ) from e
