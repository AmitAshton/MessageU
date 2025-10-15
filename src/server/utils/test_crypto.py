from utils.crypto_utils import CryptoUtils

def test_rsa():
    priv, pub = CryptoUtils.generate_rsa_keys()
    message = b"secret_message"
    enc = CryptoUtils.rsa_encrypt(pub, message)
    dec = CryptoUtils.rsa_decrypt(priv, enc)
    print("RSA OK:", dec == message)

def test_aes():
    key = CryptoUtils.generate_aes_key()
    data = b"ConfidentialData123"
    enc = CryptoUtils.aes_encrypt(key, data)
    dec = CryptoUtils.aes_decrypt(key, enc)
    print("AES OK:", dec == data)

if __name__ == "__main__":
    test_rsa()
    test_aes()
