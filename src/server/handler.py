import struct

class RequestHandler:
    def __init__(self, conn, addr):
        self.conn = conn
        self.addr = addr

    def process(self):
        try:
            header = self.conn.recv(23)  # גודל כותרת לפי ההגדרה בממן
            if not header:
                return
            # כאן יבוא פענוח לפי הפרוטוקול (Client ID, Version, Code, Payload Size)
            print(f"Received raw header from {self.addr}: {header}")
        except Exception as e:
            print(f"Error handling {self.addr}: {e}")
        finally:
            self.conn.close()
