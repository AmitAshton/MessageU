from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
import os


class CryptoUtils:
    # ---------- RSA ----------
    @staticmethod
    def generate_rsa_keys(key_size: int = 1024):
        """Generate RSA public/private key pair."""
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=key_size,
            backend=default_backend()
        )
        public_key = private_key.public_key()
        return private_key, public_key

    @staticmethod
    def rsa_encrypt(public_key, data: bytes) -> bytes:
        """Encrypt data with a public key."""
        return public_key.encrypt(
            data,
            padding.OAEP(
                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                algorithm=hashes.SHA256(),
                label=None
            )
        )

    @staticmethod
    def rsa_decrypt(private_key, ciphertext: bytes) -> bytes:
        """Decrypt data with a private key."""
        return private_key.decrypt(
            ciphertext,
            padding.OAEP(
                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                algorithm=hashes.SHA256(),
                label=None
            )
        )

    @staticmethod
    def export_public_key(public_key) -> bytes:
        """Serialize public key to PEM format."""
        return public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )

    @staticmethod
    def export_private_key(private_key) -> bytes:
        """Serialize private key to PEM format."""
        return private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.TraditionalOpenSSL,
            encryption_algorithm=serialization.NoEncryption()
        )

    @staticmethod
    def import_public_key(pem_data: bytes):
        return serialization.load_pem_public_key(pem_data, backend=default_backend())

    @staticmethod
    def import_private_key(pem_data: bytes):
        return serialization.load_pem_private_key(pem_data, password=None, backend=default_backend())

    # ---------- AES ----------
    @staticmethod
    def generate_aes_key(length: int = 16) -> bytes:
        """Generate a random AES symmetric key."""
        return os.urandom(length)

    @staticmethod
    def aes_encrypt(key: bytes, plaintext: bytes) -> bytes:
        """Encrypt plaintext with AES-CBC, IV=0 as specified in project."""
        iv = b"\x00" * 16
        cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=default_backend())
        encryptor = cipher.encryptor()

        # PKCS7 padding
        pad_len = 16 - (len(plaintext) % 16)
        plaintext += bytes([pad_len]) * pad_len

        return encryptor.update(plaintext) + encryptor.finalize()

    @staticmethod
    def aes_decrypt(key: bytes, ciphertext: bytes) -> bytes:
        """Decrypt ciphertext with AES-CBC, IV=0 as specified in project."""
        iv = b"\x00" * 16
        cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=default_backend())
        decryptor = cipher.decryptor()
        padded_plaintext = decryptor.update(ciphertext) + decryptor.finalize()

        # Remove PKCS7 padding
        pad_len = padded_plaintext[-1]
        return padded_plaintext[:-pad_len]
