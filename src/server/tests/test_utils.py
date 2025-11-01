import pytest
from utils.config_loader import ConfigLoader
from utils.logger import ServerLogger
import os

def test_config_loader_reads_port_and_version(tmp_path):
    f = tmp_path / "myport.info"
    f.write_text("9876")
    c = ConfigLoader(config_file=str(f), base_path=str(tmp_path))
    assert c.port == 9876
    assert c.version == 2

def test_config_loader_default_port():
    c = ConfigLoader(config_file="non_existent_file.info")
    assert c.port == 1357

def test_logger_creates_file():
    logger = ServerLogger() 
    logger.info("Testing if log file exists...")
    assert os.path.exists(logger.log_file)