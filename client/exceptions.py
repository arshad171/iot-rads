class MalformedPacketException(Exception):
    pass

class InvalidCommandException(Exception):
    def __init__(self,id: bytes):
        self.id = id
    
    def __str__(self) -> str:
        return f"Invalid Command ID 0x{self.id:08x}"

class InvalidDataTypeException(Exception):
    def __init__(self,id: bytes):
        self.id = id
    
    def __str__(self) -> str:
        return f"Invalid Command ID 0x{self.id:08x}"

class SerialPortException(Exception):
    pass