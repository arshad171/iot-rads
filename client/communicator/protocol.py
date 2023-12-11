""" RADS Protocol frontend """
from PyQt6.QtCore import pyqtSignal

from ui.glob import GLOBAL_SIGNALS

from communicator.format import Packet, DataType, Command
from communicator.ports import Port, Backend

class Protocol:
    """ Implements the RADS protocol """

    def __init__(self, backend: Backend):
        self.__port = Port(backend)

        # Handler registries
        self.cmd_handlers = {}
        self.cmd_signals = {}
        self.type_handlers = {}

    def register_cmd(self, command: Command):
        """ Register a command handler for the given command """

        # Perform the handler registration
        def decorator(f):
            if command.id not in self.cmd_handlers:
                self.cmd_handlers[command.id] = [f]
            else:
                self.cmd_handlers[command.id].append(f)

        return decorator

    def hook_cmd(self, command: Command, signal: pyqtSignal):
        """ Hook a QT signal to a command """
        if command.id not in self.cmd_signals:
            self.cmd_signals[command.id] = [signal]
        else:
            self.cmd_signals[command.id].append(signal)

    def register_type(self, dtype: DataType):
        """ Register a type handler for the given type """
        type_id = dtype.value[0]

        def decorator(f):
            if type_id in self.type_handlers:
                raise ValueError("Cannot register two handlers for the same type")
            self.type_handlers[type_id] = f

        return decorator

    def handle(self) -> Packet | None:
        """ Handle any incoming packet using its designated command handlers """
        while not self.__port.inbox.empty():
            packet: Packet = self.__port.inbox.get()
            dtype = packet.dtype
            cmd = packet.command

            # If we have a data type handler use it
            data = (
                self.type_handlers[dtype.id](packet.data)
                if dtype.id in self.type_handlers
                else packet.data
            )

            # Signal every registered signal handler
            if cmd.id in self.cmd_signals:
                for signal in self.cmd_signals[cmd.id]:
                    signal.emit(data)

            # Call every registered function handler
            if cmd.id in self.cmd_handlers:
                for handler in self.cmd_handlers[cmd.id]:
                    handler(data)

            if cmd.id not in self.cmd_handlers and cmd.id not in self.cmd_signals:
                GLOBAL_SIGNALS.status_signal.emit(f"No command Handler defined for Command <{cmd.label}>")

    def send(self, packet: Packet):
        """ Send a packet to the port """
        self.__port.inbox.put(packet)
