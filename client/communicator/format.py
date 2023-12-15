""" Types, Metadata, Descriptors and Containers for the protocol """

from typing import Any
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
    CMD = (3, "Command") # Empty packet - just command
    DAT = (4, "Binary data")
    WTS = (5, "Layer weights")

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
    SET_FEATURE_VECTOR = (3,"Update feature vector")
    SET_TRAINING_VECTOR = (4, "Update feature vector (for training purposes)")
    SET_INFERENCE_VECTOR = (5, "Update feature vector (for inference purposes)")
    NO_FEATURE_VECTOR =(6, "Signal no feature vector is available at this time")
    GET_FRAME = (7, "Store picture from camera")
    SET_FRAME = (8, "Request picture from camera")
    GET_BROLL = (9, "Request a series of pictures from the camera")
    SET_BROLL = (10, "Store a series of pictures from the camera")
    GET_WEIGHTS = (11, "request layer weights")
    SET_WEIGHTS = (12, "set layer weights")
    GET_LOAD_WEIGHTS = (13, "load weights request")
    SET_LOAD_WEIGHTS = (14, "load weights response")

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
    format_payload: str = "<8sIII{}B"
    format_thin: str = "<8sIII"

    # Used to build a packet that shall be sent
    def __init__(self, data: Any, command: Command, dtype: DataType):
        self.data = data
        self.command = command
        self.dtype = dtype

    def serialize(self) -> bytes:
        """Transform the packet into a byte array ready to be transmitted"""
        if self.data is not None and not isinstance(self.data,bytes):
            raise ValueError(f"Required convertion before sending {str(type(self.data))}")

        if self.data is not None:
            return struct.pack(Packet.format_payload.format(len(self.data)), self.magic, self.command.id, len(self.data), self.dtype.id, *self.data)
        else:
            return struct.pack(Packet.format_thin, self.magic, self.command.id, 0, self.dtype.id)

    def __repr__(self) -> str:
        return f"PACKET: Type <{self.dtype.label}> Command <{self.command.label}> (Payload size 0x{len(self.data):0x})"
