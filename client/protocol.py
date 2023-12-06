from serial import Serial,PARITY_NONE,STOPBITS_ONE,EIGHTBITS
from enum import Enum
import io,struct

# Communication and encoding/decoding exceptions
from exceptions import MalformedPacketException,SerialPortException,InvalidCommandException,InvalidDataTypeException
from serial.serialutil import SerialException

# Different kinds of data that can be exchanged
class DataType(Enum):
    CMD = (0,"Command")
    LOG = (1,"Log")
    TXT = (2,"Text")
    DAT = (3,"Binary Data")
    IMG = (4,"Image")

    def __init__(self,id,label):
        self.id = id
        self.label = label

    @classmethod
    def by_id(cls,id: int):
        candidates = [m for m in cls if m.value[0] == id]

        if len(candidates) == 0:
            raise InvalidDataTypeException(id)
        return candidates[0]


# Different commands associated with each packet
class Command(Enum):
    NONE = (0,"None")
    GET_TRACK_MASK = (1,"Get railroad mask")
    SET_TRACK_MASK = (2,"Set railroad mask")
    REPORT_ANOMALY = (3,"Report Anomaly")

    def __init__(self,id,desc):
        self.id = id
        self.desc = desc

    @classmethod
    def by_id(cls,id: int):
        candidates = [m for m in cls if m.value[0] == id]

        if len(candidates) == 0:
            raise InvalidCommandException(id)
        return candidates[0]


# Protocol packet
class Packet:
    magic: bytes = "IOT-RADS".encode("ascii")
    format: str = "<8sINI{}B"

    # Used to build a packet that shall be sent
    def __init__(self,data: bytes,command: Command,dtype: DataType):
        self.data = data
        self.command = command
        self.dtype = dtype

    def serialize(self) -> bytes:
        return struct.packet(Packet.format.format(len(self.data)),self.data)

    def __repr__(self) -> str:
        return f"PACKET: Type <{self.dtype.label}> Command <{self.command.desc}> (Payload size 0x{len(self.data):0x})"


class SerialPort:
    def __init__(self,port: str = None,baud: int = 9600,timeout: int = 1):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self.open = False

    # Handle "with" environment
    def __enter__(self):
        try:
            self.serial = Serial(self.port,self.baud,timeout=self.timeout,parity=PARITY_NONE,bytesize=EIGHTBITS,stopbits=STOPBITS_ONE)
            self.open = True
            return self
        except SerialException:
            raise SerialPortException("Unable to open serial port")

    def __exit__(self,exc_type,exc_value,traceback):
        if self.open:
            try:
                self.serial.close()
                self.open = False
            except SerialException:
                raise SerialPortException("Unable to  close serial port")

    def send_packet(self,data: bytes,command: Command,dtype: DataType):
        try:
            self.serial.write(Packet(data,command,dtype).serialize())
        except SerialException:
            raise SerialPortException("Unable to send data through the serial port")

    def read_packet(self) -> Packet:
        try:
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

            return Packet(payload,Command.by_id(cmd_id),DataType.by_id(type_id))
        except SerialException:
            raise SerialPortException("An error occurred while trying to read data from the serial port")


