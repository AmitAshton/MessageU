import socket
import uuid

from protocol.header import RequestHeader
from protocol.parser import RequestParser
from protocol.builder import ResponseBuilder
from protocol.enums import RequestCode, MessageType
from protocol.payload import PayloadParser
from storage.db_manager import DatabaseManager
from models.client import ClientRecord
from models.message import MessageRecord
from utils.logger import ServerLogger
from utils.crypto_utils import CryptoUtils


class RequestHandler:
    BUFFER_SIZE = 4096

    def __init__(self, conn: socket.socket, addr):
        self.conn = conn
        self.addr = addr
        self.db = DatabaseManager("defensive.db")
        self.logger = ServerLogger()
        # Each server instance can have its own private key
        # (In production, you'd persist this to disk)
        self.private_key, self.public_key = CryptoUtils.generate_rsa_keys()

    def process(self):
        try:
            while True:
                header_data = self._recv_exact(23)
                if not header_data:
                    self.logger.info(f"Client {self.addr} disconnected.")
                    break

                try:
                    header = RequestHeader.from_bytes(header_data)
                    payload = self._read_payload(header_data)

                except Exception as parse_err:
                    self.logger.error(f"Malformed request from {self.addr}: {parse_err}")
                    self.conn.sendall(ResponseBuilder.build_error())
                    continue

                self.logger.info(f"Received request code={header.code} from {header.client_id}")
                try:
                    response_bytes = self._route_request(header.code, header.client_id, payload)
                    if response_bytes:
                        self.conn.sendall(response_bytes)
                except Exception as handler_err:
                    self.logger.exception(f"Handler error: {handler_err}")
                    self.conn.sendall(ResponseBuilder.build_error())
        except ConnectionResetError:
            self.logger.warning(f"Connection reset by {self.addr}")
        finally:
            self.conn.close()
            self.logger.info(f"Connection closed: {self.addr}")

    def _recv_exact(self, size: int) -> bytes:
        data = b""
        while len(data) < size:
            packet = self.conn.recv(size - len(data))
            if not packet:
                break
            data += packet
        return data

    def _read_payload(self, header_data: bytes) -> bytes:
        payload_size = int.from_bytes(header_data[-4:], "little")
        if payload_size == 0:
            return b""
        return self._recv_exact(payload_size)

    def _route_request(self, code: int, client_id: uuid.UUID, payload):
        if code == RequestCode.REGISTER:
            return self._handle_register(payload)
        elif code == RequestCode.CLIENT_LIST:
            return self._handle_client_list(client_id)
        elif code == RequestCode.PUBLIC_KEY:
            return self._handle_public_key(payload)
        elif code == RequestCode.SEND_MESSAGE:
            return self._handle_send_message(client_id, payload)
        elif code == RequestCode.PULL_MESSAGES:
            return self._handle_pull_messages(client_id)
        else:
            self.logger.error(f"Unknown request code {code}")
            return ResponseBuilder.build_error()

    # ---------- Individual Handlers ----------

    def _handle_register(self, payload: bytes):
        # Payload = username (null-terminated) + DER public key
        try:
            null_index = payload.index(0)
            username = payload[:null_index].decode()
            public_key = payload[null_index + 1:]  # raw bytes after \0
        except Exception:
            self.logger.error("Malformed registration payload")
            return ResponseBuilder.build_error()

        existing = self.db.get_client_by_username(username)
        if existing:
            self.logger.error(f"Registration failed: username {username} already exists")
            return ResponseBuilder.build_error()

        new_client = ClientRecord(username, public_key)
        self.db.add_client(new_client)
        self.logger.info(f"Registered new client: {username} ({new_client.id})")
        return ResponseBuilder.build_register_success(new_client.id)

    def _handle_public_key(self, payload: bytes):
        try:
            target_id = uuid.UUID(bytes=payload[:16])
        except Exception:
            self.logger.error("Invalid public key request payload")
            return ResponseBuilder.build_error()

        client = self.db.get_client_by_id(target_id)
        if not client:
            self.logger.error(f"Public key request failed: client {target_id} not found")
            return ResponseBuilder.build_error()

        self.logger.info(f"Returned public key for {client.username} ({client.id})")
        return ResponseBuilder.build_public_key(client.id, client.public_key)

    def _handle_send_message(self, sender_id: uuid.UUID, payload: bytes):
        from protocol.payload import PayloadParser
        dest_id, msg_type, content = PayloadParser.parse_send_message_payload(payload)

        dest_client = self.db.get_client_by_id(dest_id)
        if not dest_client:
            self.logger.error(f"Send failed: destination {dest_id} not found")
            return ResponseBuilder.build_error()

        # Server just stores the message as received
        message = MessageRecord(dest_id, sender_id, msg_type, content)
        self.db.save_message(message)
        self.logger.info(f"Stored message from {sender_id} to {dest_id} (type={msg_type})")
        message_id = int(message.id.int & 0xFFFFFFFF)
        return ResponseBuilder.build_message_stored(dest_id, message_id)

    def _handle_pull_messages(self, client_id: uuid.UUID):
        pending = self.db.get_pending_messages(client_id)
        if not pending:
            self.logger.debug(f"No pending messages for {client_id}")
            return ResponseBuilder.build_message_stored(client_id, 0)

        messages_bytes = b""
        for msg in pending:
            msg_bytes = (
                msg.from_client.bytes +
                (int(msg.id.int & 0xFFFFFFFF)).to_bytes(4, "little") +
                msg.msg_type.to_bytes(1, "little") +
                len(msg.content).to_bytes(4, "little") +
                msg.content
            )
            messages_bytes += msg_bytes
            self.db.delete_message(msg.id)

        return ResponseBuilder.build_pending_messages(messages_bytes)
