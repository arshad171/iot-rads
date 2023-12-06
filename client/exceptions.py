class MalformedPacketException(Exception):
    pass

class InvalidCommandException(Exception):
    def __init__(self,cid: bytes):
        self.id = cid
    
    def __str__(self) -> str:
        return f"Invalid Command ID 0x{self.id:08x}"

class InvalidDataTypeException(Exception):
    def __init__(self,tid: bytes):
        self.id = tid
    
    def __str__(self) -> str:
        return f"Invalid Command ID 0x{self.id:08x}"

class SerialPortException(Exception):
    pass