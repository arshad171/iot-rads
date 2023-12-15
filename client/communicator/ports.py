""" Communication ports for the RADS network """

from abc import ABC,abstractmethod
from threading import Thread,Event
from queue import SimpleQueue
import struct
import time

from serial import Serial,SerialException,EIGHTBITS,STOPBITS_ONE,PARITY_NONE
from communicator.format import Packet,DataType,Command

from ui.glob import GLOBAL_SIGNALS

from exceptions import (
    MalformedPacketException,
    InvalidCommandException,
    InvalidDataTypeException,
    SerialPortException
)

class Backend(ABC):
    """ Abstract implementation of a P2P communication backend """
    @abstractmethod
    def open(self):
        """ Opens the channel """

    @abstractmethod
    def close(self):
        """ Closes the channel """

    @abstractmethod
    def abort(self):
        """ Aborts any connection attempt """

    @abstractmethod
    def send_packet(self, packet: Packet):
        """ Sends a packet to the destination """

    @abstractmethod
    def read_packet(self) -> Packet | None:
        """ Reads a packet from the network (if any is received within timeout) """

    @abstractmethod
    def get_timeout(self) -> float:
        """ Returns the I/O timeout of the channel """


class SerialChannel(Backend):
    """ Implements a serial backend """
    def __init__(self, path: str = None, baud: int = 9600, timeout: float = 1):
        self.__abort_event = Event()

        if path is None:
            raise ValueError("Invalid path specified for the serial port")

        self.__path = path
        self.__baud = baud
        self.__tout = timeout
        self.__port = Serial()

    def open(self):
        GLOBAL_SIGNALS.status_signal.emit("Opening Serial Port")
        self.__abort_event.clear()

        while not self.__port.is_open and not self.__abort_event.is_set():
            try:
                self.__port = Serial(self.__path,self.__baud,timeout=self.__tout,write_timeout=100,parity=PARITY_NONE,
                                     bytesize=EIGHTBITS,stopbits=STOPBITS_ONE)
                self.__port.flush()
                continue
            except SerialException:
                pass

        # Clear the window
        GLOBAL_SIGNALS.status_signal.emit("Serial Port open")

    def close(self):
        try:
            self.__port.close()
            GLOBAL_SIGNALS.status_signal.emit("Serial Port closed")
        except SerialException:
            GLOBAL_SIGNALS.status_signal.emit("Failed to close serial port.")

    def abort(self):
        self.__abort_event.set()

    def send_packet(self, packet: Packet):
        try:
            l = self.__port.write(packet.serialize())
            print(f"Sent {l} byte{'s' if l != 1 else ''}")
        except SerialException as e:
            raise SerialPortException("Unable to send data through the serial port") from e

    def read_packet(self) -> Packet | None:
        try:
            # Read the stream until we find a MAGIC
            lead = self.__port.read_until(expected=Packet.magic)

            # Validate that we actually got a good MAGIC
            if len(lead) == 0:
                return None
            if not lead.endswith(Packet.magic):
                raise MalformedPacketException(f"Invalid MAGIC received ({lead.decode('ascii')})")

            header_format = "<III"  # (uint,size_t,uint)
            header_data = self.__port.read(struct.calcsize(header_format))

            if len(header_data) != struct.calcsize(header_format):
                raise MalformedPacketException("Invalid header received")

            # Get the header fields
            cmd_id, data_size, type_id = struct.unpack(header_format, header_data)

            # Now we can read the data
            if data_size > 0:
                payload = self.__port.read(data_size)
                if len(payload) != data_size:
                    raise MalformedPacketException("Incomplete data stream")
            else:
                payload = None

            return Packet(payload, Command.by_id(cmd_id), DataType.by_id(type_id))
        except SerialException as e:
            raise SerialPortException("Error while trying to read data from the serial port") from e

    def get_timeout(self) -> float:
        return self.__tout


class Port(Thread):
    """ Implements a communication channel for the protocol """
    def __init__(self, backend: Backend):
        self.__backend = backend

        # The two queues act like letterboxes
        self.inbox = SimpleQueue()
        self.outbox = SimpleQueue()

        # Thread stop signaling logic
        self.__stop_event = Event()
        self.__is_stopped = False

        # Start the thread
        super().__init__(name="Serial Port Connection")
        self.start()

    def run(self):
        # Clear the stop event in case we are resuming
        self.__stop_event.clear()

        # Open the communication channel
        self.__backend.open()

        while not self.__stop_event.is_set():
            # Check whether we have packets to send
            while not self.outbox.empty():
                self.__backend.send_packet(self.outbox.get(False))
            try:
                if (p := self.__backend.read_packet()) is not None:
                    self.inbox.put(p)
            except InvalidDataTypeException as e:
                GLOBAL_SIGNALS.status_signal.emit(f"Received data of invalid type 0x{e.id:08x}")
            except InvalidCommandException as e:
                GLOBAL_SIGNALS.status_signal.emit(f"Received invalid command 0x{e.id:08x}")
            except MalformedPacketException as e:
                GLOBAL_SIGNALS.status_signal.emit(f"Received malformed packet: {e}")
            except SerialPortException:
                GLOBAL_SIGNALS.status_signal.emit("Serial communication crashed: restarting...")
                self.__backend.close()
                self.__backend.open()
            except Exception as e: # pylint: disable=broad-except
                GLOBAL_SIGNALS.status_signal.emit("Unknown exception while handling serial communication!")
                print(e)
                self.stop()

            # Take a break!
            time.sleep(self.__backend.get_timeout())


        # Close the serial port and signal we are ready
        self.__backend.close()
        self.__is_stopped = True

    def stop(self):
        """ Stops the mailbox """
        self.__stop_event.set()

        # In case the backend is attempting connection
        self.__backend.abort()

    def stopped(self) -> bool:
        """ Returns whether the mailbox has stopped and the port has been closed """
        return self.__is_stopped
