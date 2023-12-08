""" Types, Metadata, Descriptors and Containers for the protocol """

from enum import Enum
import struct

from exceptions import (
    InvalidCommandException,
    InvalidDataTypeException,
)


class DataType(Enum):
    """ Type of data being transmitted """

    LOG = (0, "Log")
    MAT = (1, "Matrix")
    IMG = (2, "Image")

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
    WRITE_LOG = (0, "Write logs")
    REPORT_ANOMALY = (1, "Report anomaly")
    GET_FEATURE_VECTOR = (2, "Request feature vector")
    SET_FEATURE_VECTOR = (3, "Update feature vector")
    GET_FRAME = (4, "Get picture from camera")
    SET_FRAME = (5, "Store picture from camera")

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
    """ Protocol packet """

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
