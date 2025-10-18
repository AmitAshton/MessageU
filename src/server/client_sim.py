import socket
import uuid
import struct
from utils.crypto_utils import CryptoUtils

# ===== Constants =====
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 1234

REQ_REGISTER = 600
REQ_CLIENT_LIST = 601
REQ_PUBLIC_KEY = 602
REQ_SEND_MESSAGE = 603
REQ_PULL_MESSAGES = 604

MSG_REQUEST_SYM_KEY = 1
MSG_SEND_SYM_KEY = 2
MSG_TEXT_MESSAGE = 3

VERSION = 2


# ===== Helpers =====
def recv_all(sock, size):
    data = b""
    while len(data) < size:
        packet = sock.recv(size - len(data))
        if not packet:
            break
        data += packet
    return data


def build_request(client_id: uuid.UUID, code: int, payload: bytes):
    payload_size = len(payload)
    header = (
        client_id.bytes +
        VERSION.to_bytes(1, "little") +
        code.to_bytes(2, "little") +
        payload_size.to_bytes(4, "little")
    )
    return header + payload


def send_request(sock, client_id, code, payload=b""):
    req = build_request(client_id, code, payload)
    sock.sendall(req)
    header = recv_all(sock, 7)  # version(1) + code(2) + size(4)
    if not header:
        raise ConnectionError("No response header from server")
    ver, resp_code, size = header[0], int.from_bytes(header[1:3], "little"), int.from_bytes(header[3:], "little")
    data = recv_all(sock, size) if size else b""
    return ver, resp_code, data


def connect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((SERVER_HOST, SERVER_PORT))
    return s


# ===== Simulation Flow =====
def register_user(name: str, public_key: bytes):
    s = connect()
    payload = name.encode() + b"\0" + public_key
    v, code, data = send_request(s, uuid.UUID(int=0), REQ_REGISTER, payload)
    s.close()
    client_id = uuid.UUID(bytes=data) if data else None
    print(f"[REGISTER] {name} -> {code}, ID={client_id}")
    return client_id


def get_public_key(requester_id, target_id):
    s = connect()
    v, code, data = send_request(s, requester_id, REQ_PUBLIC_KEY, target_id.bytes)
    s.close()
    # Skip the first 16 bytes (UUID)
    return data[16:]


def send_message(sender_id, dest_id, msg_type, content):
    s = connect()
    payload = dest_id.bytes + msg_type.to_bytes(1, "little") + content
    v, code, data = send_request(s, sender_id, REQ_SEND_MESSAGE, payload)
    s.close()
    print(f"[SEND] {msg_type} -> {code}")
    return code


def pull_messages(client_id):
    s = connect()
    v, code, data = send_request(s, client_id, REQ_PULL_MESSAGES)
    s.close()
    print(f"[PULL] -> {code}, {len(data)} bytes")
    return data


def run_full_encryption_test():
    priv_a, pub_a_obj = CryptoUtils.generate_rsa_keys()
    priv_b, pub_b_obj = CryptoUtils.generate_rsa_keys()

    # serialize to DER bytes so the server can store them
    pub_a = CryptoUtils.export_public_key(pub_a_obj)
    pub_b = CryptoUtils.export_public_key(pub_b_obj)

    alice_id = register_user("Alice", pub_a)
    bob_id = register_user("Bob", pub_b)

    # Alice requests Bob’s public key
    bob_pub = get_public_key(alice_id, bob_id)
    print(f"[GET_KEY] Bob’s key size: {len(bob_pub)} bytes")

    # Encrypt symmetric key and send from Alice → Bob
    sym_key = CryptoUtils.generate_aes_key()
    pub_key_obj = CryptoUtils.import_public_key(bob_pub)
    encrypted_key = CryptoUtils.rsa_encrypt(pub_key_obj, sym_key)

    send_message(alice_id, bob_id, MSG_SEND_SYM_KEY, encrypted_key)

    # Bob pulls messages
    pending = pull_messages(bob_id)
    if pending:
        print("[MESSAGE RETRIEVED] Bob received encrypted data.")
    else:
        print("[MESSAGE RETRIEVED] No data found.")


if __name__ == "__main__":
    run_full_encryption_test()
