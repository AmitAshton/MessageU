from enum import IntEnum

class RequestCode(IntEnum):
    REGISTER = 600
    CLIENT_LIST = 601
    PUBLIC_KEY = 602
    SEND_MESSAGE = 603
    PULL_MESSAGES = 604


class ResponseCode(IntEnum):
    REGISTER_SUCCESS = 2100
    CLIENT_LIST = 2101
    PUBLIC_KEY = 2102
    MESSAGE_STORED = 2103
    PENDING_MESSAGES = 2104
    GENERAL_ERROR = 9000


class MessageType(IntEnum):
    REQUEST_SYM_KEY = 1
    SEND_SYM_KEY = 2
    TEXT_MESSAGE = 3
    FILE_MESSAGE = 4


class ProtocolVersion(IntEnum):
    SERVER = 2
    CLIENT = 1
