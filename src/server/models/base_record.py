from abc import ABC, abstractmethod
from typing import Dict, Any

class BaseRecord(ABC):
    @abstractmethod
    def to_dict(self) -> Dict[str, Any]:
        pass

    @abstractmethod
    def from_dict(self, data: Dict[str, Any]):
        pass
