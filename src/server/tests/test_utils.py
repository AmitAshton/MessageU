import pytest
from utils.crypto_utils import CryptoUtils
from utils.config_loader import ConfigLoader
from utils.logger import ServerLogger


def test_rsa_key_generation_and_usage(crypto: CryptoUtils):
    priv, pub = crypto.generate_rsa_keys()
    msg = b"secure"
    enc = crypto.rsa_encrypt(pub, msg)
    dec = crypto.rsa_decrypt(priv, enc)
    assert dec == msg

def test_aes_encrypt_decrypt(crypto: CryptoUtils):
    key = crypto.generate_aes_key()
    data = b"HelloWorld"
    enc = crypto.aes_encrypt(key, data)
    dec = crypto.aes_decrypt(key, enc)
    assert dec == data

def test_invalid_aes_key_length(crypto: CryptoUtils):
    with pytest.raises(ValueError):
        crypto.aes_encrypt(b"short", b"data")

def test_config_loader_reads_port_and_version(tmp_path):
    f = tmp_path / "myport.info"
    f.write_text("9876")
    c = ConfigLoader(base_path=str(tmp_path))
    assert c.port == 9876
    assert c.version in [1, 2]

def test_logger_creates_file(tmp_path):
    log_path = tmp_path / "log.log"
    logger = ServerLogger(str(log_path))
    logger.info("hello")
    assert log_path.exists()
