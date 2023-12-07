""" Definition of the communication protocol for the RADS network """

from enum import Enum
import struct

from communicator.ports import SerialPort
from exceptions import (
    InvalidCommandException,
    InvalidDataTypeException,
)


class DataType(Enum):
    """ Type of data being transmitted """

    CMD = (0, "Command")
    LOG = (1, "Log")
    TXT = (2, "Text")
    DAT = (3, "Binary Data")
    IMG = (4, "Image")

    def __init__(self, tid, label):
        self.id = tid
        self.label = label

    @classmethod
    def by_id(cls, tid: int):
        """ Get a DataType object by ID """
        candidates = [m for m in cls if m.value[0] == tid]

        if len(candidates) == 0:
            raise InvalidDataTypeException(tid)
        return candidates[0]


class Command(Enum):
    """ Action/Notification associated with each command """

    NONE = (0, "None")
    GET_TRACK_MASK = (1, "Get railroad mask")
    SET_TRACK_MASK = (2, "Set railroad mask")
    REPORT_ANOMALY = (3, "Report Anomaly")

    def __init__(self, cid, label):
        self.id = cid
        self.label = label

    @classmethod
    def by_id(cls, cid: int):
        """Get a Command object by ID"""
        candidates = [m for m in cls if m.value[0] == cid]

        if len(candidates) == 0:
            raise InvalidCommandException(cid)
        return candidates[0]


class Packet:
    """ Network communication packet """

    magic: bytes = "IOT-RADS".encode("ascii")
    format: str = "<8sINI{}B"

    # Used to build a packet that shall be sent
    def __init__(self, data: bytes, command: Command, dtype: DataType):
        self.data = data
        self.command = command
        self.dtype = dtype

    def serialize(self) -> bytes:
        """Transform the packet into a byte array ready to be transmitted"""
        return struct.pack(Packet.format.format(len(self.data)), self.data)

    def __repr__(self) -> str:
        return f"PACKET: Type <{self.dtype.label}> Command <{self.command.label}> (Payload size 0x{len(self.data):0x})"


class Protocol:
    """ Implements the RADS protocol """

    def __init__(self, port: str, baud: int = 9600, retry: float = 1):
        self.serial = SerialPort(port,baud,retry)

        # Handler registries
        self.cmd_handlers = {}
        self.type_handlers = {}

    def register_cmd(self, command: Command):
        """ Register a command handler for a given command """
        cmd_id = command.value[0]

        # Perform the handler registration
        def decorator(f):
            if cmd_id not in self.cmd_handlers:
                self.cmd_handlers[cmd_id] = [f]
            else:
                self.cmd_handlers[cmd_id].append(f)

        return decorator

    def register_type(self, dtype: DataType):
        """ Register a type handler for a given type """
        type_id = dtype.value[0]

        if type_id in self.type_handlers:
            raise ValueError("Cannot register two handlers for the same type")
        else:
            self.type_handlers[type_id] = f

    def handle(self) -> Packet | None:
        """ Handle any incoming packet using command handlers """
        while not self.serial.inbox.empty():
            packet: Packet = self.serial.inbox.get()
            dtype = packet.dtype
            cmd = packet.command

            # If we have a data type handler use it
            data = (
                self.type_handlers[dtype.id](packet.data)
                if dtype.id in self.type_handlers
                else data
            )

            # Call every registered function handler
            if cmd.id in self.cmd_handlers:
                for handler in self.cmd_handlers:
                    handler(data)
            else:
                print(f"******** No command Handler defined for Command <{cmd.label}>")

    def send(self, packet: Packet):
        """ Send a packet to the device """
        self.serial.inbox.put(packet)
