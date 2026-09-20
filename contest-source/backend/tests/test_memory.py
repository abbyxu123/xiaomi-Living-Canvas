from test_handoff import prepare_candidate


def hand_off(client):
    session_id = prepare_candidate(client)
    client.post(
        "/v1/device/event",
        json={"session_id": session_id, "event": "accept_candidate"},
    )
    client.post(
        "/v1/confirm", json={"session_id": session_id, "platform": "eleme"}
    )
    return session_id


def test_feedback_requires_a_confirmed_handoff(client):
    session_id = prepare_candidate(client)
    response = client.post(
        "/v1/feedback",
        json={"session_id": session_id, "liked": True, "note": "好吃"},
    )
    assert response.status_code == 409
    assert response.json()["detail"] == "confirmed_choice_required"


def test_confirmed_feedback_can_be_listed(client):
    session_id = hand_off(client)
    saved = client.post(
        "/v1/feedback",
        json={"session_id": session_id, "liked": True, "note": "下次还想吃"},
    )
    assert saved.status_code == 201

    memory = client.get("/v1/memory").json()
    assert memory["feedback"][0]["candidate_id"]
    assert memory["feedback"][0]["liked"] is True
    assert memory["feedback"][0]["note"] == "下次还想吃"


def test_temporary_preferences_are_not_promoted_automatically(client):
    session_id = hand_off(client)
    client.post(
        "/v1/feedback",
        json={
            "session_id": session_id,
            "liked": True,
            "temporary_preferences": ["今天少辣"],
        },
    )
    assert client.get("/v1/memory").json()["remembered_preferences"] == []


def test_explicit_preferences_can_be_remembered(client):
    session_id = hand_off(client)
    client.post(
        "/v1/feedback",
        json={
            "session_id": session_id,
            "liked": True,
            "remember_preferences": ["偏爱米饭"],
        },
    )
    assert client.get("/v1/memory").json()["remembered_preferences"] == [
        "偏爱米饭"
    ]


def test_all_profile_memory_can_be_deleted(client):
    session_id = hand_off(client)
    client.post(
        "/v1/feedback",
        json={
            "session_id": session_id,
            "liked": False,
            "remember_preferences": ["不喜欢太甜"],
        },
    )
    deleted = client.delete("/v1/memory")
    assert deleted.status_code == 200
    assert deleted.json()["deleted"] is True
    assert client.get("/v1/memory").json() == {
        "feedback": [],
        "remembered_preferences": [],
    }
