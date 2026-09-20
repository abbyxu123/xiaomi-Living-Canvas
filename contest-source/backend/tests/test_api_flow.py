def create_session(client):
    response = client.post("/v1/session", json={})
    assert response.status_code == 201
    return response.json()["session_id"]


def valid_input(session_id):
    return {
        "session_id": session_id,
        "context": {"people": 1, "occasion": "晚餐"},
        "hard_constraints": {
            "max_total_price_cny": 50,
            "max_delivery_minutes": 30,
            "channel": "delivery",
            "allergens": [],
            "diet_taboos": [],
            "dislikes": [],
        },
        "soft_preferences": {
            "cuisines": ["中式"],
            "temperatures": ["热"],
            "preferred_tags": ["米饭"],
            "novelty": "balanced",
        },
    }


def test_creates_opaque_bounded_session(client):
    response = client.post("/v1/session", json={"device_id": "gemini-s1-demo"})
    body = response.json()
    assert response.status_code == 201
    assert body["session_id"].startswith("meal_")
    assert len(body["session_id"]) == 17
    assert body["state"] == "created"


def test_input_returns_structured_rules_candidate(client):
    session_id = create_session(client)
    response = client.post("/v1/input", json=valid_input(session_id))
    body = response.json()

    assert response.status_code == 200
    assert body["session_id"] == session_id
    assert body["state"] == "candidate"
    assert body["recommendation"]["source"] == "rules"
    assert body["recommendation"]["rules_fallback"] is True
    assert body["recommendation"]["candidate"]["search_query"]


def test_input_rejects_unknown_session(client):
    response = client.post("/v1/input", json=valid_input("meal_000000000000"))
    assert response.status_code == 404
    assert response.json()["detail"] == "session_not_found"


def test_input_requires_budget_time_and_channel(client):
    session_id = create_session(client)
    payload = valid_input(session_id)
    payload["hard_constraints"].pop("max_total_price_cny")
    response = client.post("/v1/input", json=payload)
    assert response.status_code == 422


def test_rules_only_mode_is_repeatable(client):
    first = create_session(client)
    second = create_session(client)
    first_choice = client.post("/v1/input", json=valid_input(first)).json()
    second_choice = client.post("/v1/input", json=valid_input(second)).json()
    assert (
        first_choice["recommendation"]["candidate"]["candidate_id"]
        == second_choice["recommendation"]["candidate"]["candidate_id"]
    )
