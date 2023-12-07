""" Definition of the communication protocol for the RADS network """

from enum import Enum
import struct

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
