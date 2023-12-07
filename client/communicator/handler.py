from communicator.protocol import Packet, DataType, Command
from communicator.ports import SerialPort

class ProtocolHandler:
    """ Implements the RADS protocol """

    def __init__(self, port: str, baud: int = 9600, interval: float = 1):
        self.serial = SerialPort(port,baud,interval)

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

        def decorator(f):
            if type_id in self.type_handlers:
                raise ValueError("Cannot register two handlers for the same type")
            self.type_handlers[type_id] = f

        return decorator

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
                else packet.data
            )

            # Call every registered function handler
            if cmd.id in self.cmd_handlers:
                for handler in self.cmd_handlers[cmd.id]:
                    handler(data)
            else:
                print(f"******** No command Handler defined for Command <{cmd.label}>")

    def send(self, packet: Packet):
        """ Send a packet to the device """
        self.serial.inbox.put(packet)
