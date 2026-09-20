from urllib.parse import parse_qs, urlparse

import pytest

from living_canvas_backend.handoff import build_platform_search_url, is_allowed_url
from test_api_flow import create_session, valid_input


def prepare_candidate(client):
    session_id = create_session(client)
    assert client.post("/v1/input", json=valid_input(session_id)).status_code == 200
    return session_id


def test_accept_then_confirm_creates_phone_handoff(client):
    session_id = prepare_candidate(client)
    accepted = client.post(
        "/v1/device/event",
        json={"session_id": session_id, "event": "accept_candidate"},
    )
    assert accepted.status_code == 200
    assert accepted.json()["state"] == "confirming"

    confirmed = client.post(
        "/v1/confirm", json={"session_id": session_id, "platform": "eleme"}
    )
    body = confirmed.json()
    assert confirmed.status_code == 200
    assert body["state"] == "handed_off"
    assert is_allowed_url(body["external_url"])
    assert body["compact_path"].startswith("/c/")
    assert body["requires_user_payment"] is True


def test_confirm_before_device_accept_is_rejected(client):
    session_id = prepare_candidate(client)
    response = client.post(
        "/v1/confirm", json={"session_id": session_id, "platform": "meituan"}
    )
    assert response.status_code == 409
    assert response.json()["detail"] == "confirmation_required"


def test_platform_url_is_allowlisted_and_query_is_encoded():
    url = build_platform_search_url("eleme", "鸡肉 盖饭&汤")
    parsed = urlparse(url)
    assert parsed.hostname == "www.ele.me"
    assert parse_qs(parsed.query)["keyword"] == ["鸡肉 盖饭&汤"]
    assert is_allowed_url(url)
    assert not is_allowed_url("https://www.ele.me.example.com/search")
    with pytest.raises(ValueError):
        build_platform_search_url("unknown", "盖饭")


def test_compact_link_redirects_to_owned_console(client):
    session_id = create_session(client)
    token = session_id.removeprefix("meal_")
    response = client.get("/c/" + token, follow_redirects=False)
    assert response.status_code == 307
    assert response.headers["location"] == "/console?session=" + session_id
    assert client.get("/c/not-valid", follow_redirects=False).status_code == 404


def test_phone_console_has_living_canvas_roles(client):
    response = client.get("/console")
    assert response.status_code == 200
    assert "Living Canvas" in response.text
    assert "画间" in response.text
    for role in ["比格犬", "美短猫", "鸽子", "小猪", "外星人"]:
        assert role in response.text
