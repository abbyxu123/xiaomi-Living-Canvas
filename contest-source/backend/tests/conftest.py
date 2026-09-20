import pytest
from fastapi.testclient import TestClient

from living_canvas_backend.app import create_app


@pytest.fixture
def client(tmp_path):
    return TestClient(create_app(data_dir=tmp_path))
