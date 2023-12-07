""" Communication ports for the RADS network """

from threading import Thread,Event
from queue import SimpleQueue
import struct
import time

from serial import Serial,SerialException,EIGHTBITS,STOPBITS_ONE,PARITY_NONE
from communicator.protocol import Packet,DataType,Command

from exceptions import (
    MalformedPacketException,
    InvalidCommandException,
    InvalidDataTypeException,
    SerialPortException
)


class SerialPort(Thread):
    """ Implements a serial port (together with the RADS protocol) """
    def __init__(self, port: str, baud: int, interval: float):
        # Store serial port parameters
        self.__port = port
        self.__baud = baud
        self.__interval = interval

        # The two queues act like letterboxes
        self.inbox = SimpleQueue()
        self.outbox = SimpleQueue()

        # Initialize a dummy serial object
        self.__serial = Serial()

        # Thread stop signaling logic
        self.__stop_event = Event()
        self.__is_stopped = False

        # Start the thread
        super().__init__(name="Serial Port Connection")
        self.start()

    def __open_serial(self):
        print("******** Opening Serial Port")
        while not self.__serial.is_open:
            try:
                self.__serial = Serial(self.__port,self.__baud,timeout=self.__interval,
                                       parity=PARITY_NONE,bytesize=EIGHTBITS,stopbits=STOPBITS_ONE)
                continue
            except SerialException:
                time.sleep(self.__interval)

    def __close_serial(self):
        try:
            self.__serial.close()
        except SerialException:
            print("******** Failed to close serial port.")

    def run(self):
        # Clear the stop event in case we are resuming
        self.__stop_event.clear()

        # Open the serial port
        self.__open_serial()

        while not self.__stop_event.is_set():
            # Check whether we have packets to send
            while not self.outbox.empty():
                self.send_packet(self.outbox.get(False))

            try:
                if (p := self.read_packet()) is not None:
                    self.inbox.put(p)
            except InvalidDataTypeException as e:
                print(f"******** Received data of invalid type 0x{e.id:08x}")
            except InvalidCommandException as e:
                print(f"******** Received invalid command 0x{e.id:08x}")
            except MalformedPacketException as e:
                print(f"******** Received malformed packet: {e}")
            except SerialPortException:
                print("******** Serial communication crashed: restarting...")
                self.__close_serial()
                self.__open_serial()
            except Exception as e:
                print("******** Unknown exception while handling serial communication!")
                print(e)

                self.stop()
            
            # Take a break!
            time.sleep(self.__interval)


        # Close the serial port and signal we are ready
        self.__close_serial()
        self.__is_stopped = True

    def stop(self):
        """ Stop the thread """
        self.__stop_event.set()

    def stopped(self) -> bool:
        """ Wether the thread has stopped """
        return self.__is_stopped

    def send_packet(self, packet: Packet):
        """ Send a packet over the serial port """
        try:
            self.__serial.write(packet.serialize())
        except SerialException as e:
            raise SerialPortException("Unable to send data through the serial port") from e

    def read_packet(self) -> Packet:
        """ Receive a packet from the serial port (if any) """
        try:
            # Read the stream until we find a MAGIC
            lead = self.__serial.read_until(expected=Packet.magic)

            # Validate that we actually got a good MAGIC
            if len(lead) == 0:
                return None
            if not lead.endswith(Packet.magic):
                raise MalformedPacketException("Invalid MAGIC received")

            header_format = "<III"  # (uint,size_t,uint)
            header_data = self.__serial.read(struct.calcsize(header_format))

            if len(header_data) != struct.calcsize(header_format):
                raise MalformedPacketException("Invalid header received")

            # Get the header fields
            cmd_id, data_size, type_id = struct.unpack(header_format, header_data)

            # Now we can read the data
            payload = self.__serial.read(data_size)
            if len(payload) != data_size:
                raise MalformedPacketException("Incomplete data stream")

            return Packet(payload, Command.by_id(cmd_id), DataType.by_id(type_id))
        except SerialException as e:
            raise SerialPortException(
                "An error occurred while trying to read data from the serial port"
            ) from e
