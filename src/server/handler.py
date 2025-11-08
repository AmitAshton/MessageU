import socket
import uuid

from protocol.header import RequestHeader
from protocol.builder import ResponseBuilder
from protocol.enums import RequestCode, MessageType
from protocol.payload import PayloadParser  # Ensure this parser is used
from storage.db_manager import DatabaseManager
from models.client import ClientRecord
from models.message import MessageRecord
from utils.logger import ServerLogger


class RequestHandler:
    BUFFER_SIZE = 4096

    def __init__(self, conn: socket.socket, addr):
        self.conn = conn
        self.addr = addr
        self.db = DatabaseManager("defensive.db")
        self.logger = ServerLogger()

    def process(self):
        try:
            while True:
                # Per protocol spec, request header is 23 bytes
                header_data = self._recv_exact(23)
                if not header_data:
                    self.logger.info(f"Client {self.addr} disconnected.")
                    break

                try:
                    header = RequestHeader.from_bytes(header_data)
                    payload = self._read_payload(header) # Pass header object

                except Exception as parse_err:
                    self.logger.error(f"Malformed request from {self.addr}: {parse_err}")
                    self.conn.sendall(ResponseBuilder.build_error())
                    continue

                self.logger.info(f"Received request code={header.code} from {header.client_id}")
                try:
                    response_bytes = self._route_request(header, payload)
                    if response_bytes:
                        self.conn.sendall(response_bytes)
                except Exception as handler_err:
                    self.logger.exception(f"Handler error: {handler_err}")
                    self.conn.sendall(ResponseBuilder.build_error())
        except ConnectionResetError:
            self.logger.warning(f"Connection reset by {self.addr}")
        except (socket.timeout, OSError):
            self.logger.warning(f"Connection error with {self.addr}")
        finally:
            self.conn.close()
            self.logger.info(f"Connection closed: {self.addr}")

    def _recv_exact(self, size: int) -> bytes:
        data = b""
        while len(data) < size:
            packet = self.conn.recv(size - len(data))
            if not packet:
                # Connection closed prematurely
                raise ConnectionResetError("Client disconnected during read")
            data += packet
        return data

    def _read_payload(self, header: RequestHeader) -> bytes:
        payload_size = header.payload_size
        if payload_size == 0:
            return b""
        return self._recv_exact(payload_size)

    def _route_request(self, header: RequestHeader, payload: bytes):
        code = header.code
        client_id = header.client_id

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


    def _handle_register(self, payload: bytes):
        try:
            username, public_key = PayloadParser.parse_register_payload(payload)
        except Exception as e:
            self.logger.error(f"Malformed registration payload: {e}")
            return ResponseBuilder.build_error()

        existing = self.db.get_client_by_username(username)
        if existing:
            self.logger.error(f"Registration failed: username {username} already exists")
            return ResponseBuilder.build_error()

        new_client = ClientRecord(username, public_key)
        try:
            self.db.add_client(new_client)
        except ValueError as e: # Handle duplicate username race condition
             self.logger.error(f"Registration failed: {e}")
             return ResponseBuilder.build_error()
             
        self.logger.info(f"Registered new client: {username} ({new_client.id})")
        return ResponseBuilder.build_register_success(new_client.id)

    def _handle_public_key(self, payload: bytes):
        try:
            # payload is just the Client ID
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
        try:
            dest_id, msg_type, content = PayloadParser.parse_send_message_payload(payload)
        except Exception as e:
            self.logger.error(f"Malformed send message payload: {e}")
            return ResponseBuilder.build_error()

        dest_client = self.db.get_client_by_id(dest_id)
        if not dest_client:
            self.logger.error(f"Send failed: destination {dest_id} not found")
            return ResponseBuilder.build_error()

        # Validate message type
        if msg_type not in [MessageType.REQUEST_SYM_KEY, MessageType.SEND_SYM_KEY,
                            MessageType.TEXT_MESSAGE, MessageType.FILE_MESSAGE]:
            self.logger.error(f"Unsupported message type: {msg_type}")
            return ResponseBuilder.build_error()

        # Store message as-is (Stateless server)
        message = MessageRecord(dest_id, sender_id, msg_type, content)
        self.db.save_message(message)

        if msg_type == MessageType.FILE_MESSAGE:
            self.logger.info(f"Stored file message from {sender_id} to {dest_id} ({len(content)} bytes)")
        else:
            self.logger.info(f"Stored message from {sender_id} to {dest_id} (type={msg_type})")

        # Truncate UUID to 4-byte int for the protocol response
        message_id = int(message.id.int & 0xFFFFFFFF)
        return ResponseBuilder.build_message_stored(dest_id, message_id)

    def _handle_pull_messages(self, client_id: uuid.UUID):
        pending = self.db.get_pending_messages(client_id)
        if not pending:
            self.logger.debug(f"No pending messages for {client_id}")
            return ResponseBuilder.build_pending_messages(b"")

        messages_bytes = b""
        ids_to_delete = []
        for msg in pending:
            # MessageID is 4 bytes
            msg_id_bytes = (int(msg.id.int & 0xFFFFFFFF)).to_bytes(4, "little")
            
            msg_bytes = (
                msg.from_client.bytes +
                msg_id_bytes +
                msg.msg_type.to_bytes(1, "little") +
                len(msg.content).to_bytes(4, "little") +
                msg.content
            )
            messages_bytes += msg_bytes
            ids_to_delete.append(msg.id)

        # Delete messages after building the response
        for msg_id in ids_to_delete:
            self.db.delete_message(msg_id)

        self.logger.info(f"Pulled {len(pending)} messages for {client_id}")
        return ResponseBuilder.build_pending_messages(messages_bytes)

    def _handle_client_list(self, client_id: uuid.UUID):
        all_clients = self.db.list_clients()
        # list should not include the requester
        other_clients = [c for c in all_clients if c.id != client_id]

        if not other_clients:
            self.logger.info(f"Client list requested by {client_id}, no other clients found.")
            return ResponseBuilder.build_client_list([])

        self.logger.info(f"Returning client list ({len(other_clients)} clients) to {client_id}")
        return ResponseBuilder.build_client_list(other_clients)