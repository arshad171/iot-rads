from serial import Serial
from enum import Enum
import io,struct

# Communication and encoding/decoding exceptions
from exceptions import MalformedPacketException

# Different kinds of data that can be exchanged
class DataType(Enum):
    CMD = 0
    LOG = 1
    TXT = 2
    DAT = 3
    IMG = 4


# Different commands associated with each packet
class CommandType(Enum):
    NONE = 0
    GET_TRACK_MASK = 1
    SET_TRACK_MASK = 2
    REPORT_ANOMALY = 3


# Protocol packet
class Packet:
    magic: bytes = "IOT-RADS".encode("ascii")
    format: str = "<8sINI{}B"

    # Used to build a packet that shall be sent
    def __init__(self,data: bytes,command: CommandType,dtype: DataType):
        self.data = data
        self.command = command
        self.dtype = dtype
    
    def serialize(self) -> bytes:
        return struct.packet(Packet.format.format(len(self.data)),self.data)
    
    def __repr__(self) -> str:
        return f"[PACKET] Type {self.dtype} CommandType {self.command} (Payload size 0x{len(self.data):0x})"


class SerialPort:
    def __init__(self,port: str = None,baud: int = 9600,timeout: int = 1):
        self.serial = Serial(port,baud,timeout=timeout)
    
    # Handle "with" environment
    def __enter__(self):
        self.serial.open()
    
    def __exit__(self,exc_type,exc_value,traceback):
        self.serial.close()

    def send_packet(self,data: bytes,command: CommandType,dtype: DataType):
        self.serial.write(Packet(data,command,dtype).serialize())

    def read_packet(self) -> Packet:
        # Read the stream until we find a MAGIC
        lead = self.serial.read_until(expected=Packet.magic)

        # Validate that we actually got a good MAGIC
        if len(lead) == 0:
            return None
        if not  lead.endswith(Packet.magic):
            raise MalformedPacketException("Invalid MAGIC received")

        header_format = "<III" # (uint,size_t,uint)
        header_data = self.serial.read(struct.calcsize(header_format))

        if len(header_data) != struct.calcsize(header_format):
            raise MalformedPacketException("Invalid header received")
        
        # Get the header fields
        cmd_id,data_size,type_id = struct.unpack(header_format,header_data)

        # Now we can read the data
        payload = self.serial.read(data_size)
        if len(payload) != data_size:
            raise MalformedPacketException("Incomplete data stream")
        
        return Packet(payload,CommandType(cmd_id),DataType(type_id))


