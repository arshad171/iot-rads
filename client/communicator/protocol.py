""" RADS Protocol frontend """
from PyQt6.QtCore import pyqtSignal

from ui.glob import GLOBAL_SIGNALS

from communicator.format import Packet, DataType, Command
from communicator.ports import Port, Backend

class Protocol:
    """ Implements the RADS protocol """

    def __init__(self, backend: Backend | None):
        if backend is not None:
            self.__port = Port(backend)
        else:
            self.__port = None

        # Handler registries
        self.cmd_handlers = {}
        self.cmd_signals = {}
        self.decoders = {}
        self.encoders = {}

    def set_backend(self, backend: Backend):
        """ Sets the backend to the protocol """
        if self.__port is not None:
            self.__port.stop()

            
        self.__port = Port(backend)

    def has_backend(self):
        """ Checks whether a backend is connecte to this protocol handler """
        return self.__port is not None

    def register_cmd_direct(self, command: Command, f: callable):
        """ Register a command handler for the given command """
        if command.id not in self.cmd_handlers:
            self.cmd_handlers[command.id] = [f]
        else:
            self.cmd_handlers[command.id].append(f)

    def register_cmd(self, command: Command):
        """ Register a command handler for the given command """
        def decorator(f):
            self.register_cmd_direct(command,f)
        return decorator

    def hook_cmd(self, command: Command, signal: pyqtSignal):
        """ Hook a QT signal to a command """
        if command.id not in self.cmd_signals:
            self.cmd_signals[command.id] = [signal]
        else:
            self.cmd_signals[command.id].append(signal)

    def register_decoder_direct(self, dtype: DataType, f: callable):
        """ Register an encoder for the given type """
        if dtype.id in self.decoders:
            raise ValueError("Cannot register two handlers for the same type")
        self.decoders[dtype.id] = f

    def register_decoder(self, dtype: DataType):
        """ Register an encoder for the given type """
        def decorator(f):
            self.register_decoder_direct(dtype,f)
        return decorator

    def register_encoder_direct(self, dtype: DataType, f: callable):
        """ Register a decoder for the given type """
        if dtype.id in self.encoders:
            raise ValueError("Cannot register two handlers for the same type")
        self.encoders[dtype.id] = f

    def register_encoder(self, dtype: DataType):
        """ Register a decoder for the given type """
        def decorator(f):
            self.register_encoder_direct(dtype,f)
        return decorator

    def handle(self) -> Packet | None:
        """ Handle any incoming packet using its designated command handlers """
        # Check that we are initialized
        if self.__port is None:
            raise RuntimeError("Backend has not been initialized")

        while not self.__port.inbox.empty():
            packet: Packet = self.__port.inbox.get()
            dtype = packet.dtype
            cmd = packet.command

            # If we have a data type handler use it
            data = (
                self.decoders[dtype.id](packet.data)
                if dtype.id in self.decoders
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
        if packet.dtype.id in self.encoders:
            # Convert the data segment to its C representation using the decoder
            packet.data = self.encoders[packet.dtype.id](packet.data)
        self.__port.outbox.put(packet)

    def start(self):
        """ Starts the mailbox """
        self.__port.start()

    def stop(self):
        """ Stop the mailbox """
        if self.__port is None:
            return

        self.__port.stop()

        # Wait for the port to stop
        while not self.__port.stopped():
            pass

        self.__port = None
