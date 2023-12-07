""" Exception for the RADS Client """

class MalformedPacketException(Exception):
    """ The received packet structure is corrupt and data unrecoverable """

class InvalidCommandException(Exception):
    """ The received Command ID is unknown """
    def __init__(self,cid: bytes):
        self.id = cid

    def __str__(self) -> str:
        return f"Invalid Command ID 0x{self.id:08x}"

class InvalidDataTypeException(Exception):
    """ THe received DataType ID is unknown """
    def __init__(self,tid: bytes):
        self.id = tid

    def __str__(self) -> str:
        return f"Invalid Command ID 0x{self.id:08x}"

class SerialPortException(Exception):
    """ The serial communication has broke down """
