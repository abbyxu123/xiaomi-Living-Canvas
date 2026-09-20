import json

import httpx

from living_canvas_backend.model_gateway import ModelGateway
from living_canvas_backend.schema import Candidate, SoftPreferences


def candidate(candidate_id):
    return Candidate(
        candidate_id=candidate_id,
        name=candidate_id,
        cuisine="中式",
        temperature="热",
        price_cny=30,
        delivery_minutes=25,
        channels=["delivery"],
    )


def gateway(handler=None, **overrides):
    client = httpx.Client(transport=httpx.MockTransport(handler)) if handler else None
    values = {
        "base_url": "https://model.example/v1",
        "model": "decision-model",
        "api_key": "secret-value",
        "client": client,
    }
    values.update(overrides)
    return ModelGateway(**values)


def test_disabled_gateway_does_not_call_network():
    model = gateway(base_url="", model="", api_key="", client=None)
    assert model.choose([candidate("rice")], SoftPreferences()) is None


def test_timeout_falls_back_without_raising():
    def timeout(request):
        raise httpx.ReadTimeout("slow", request=request)

    assert gateway(timeout).choose([candidate("rice")], SoftPreferences()) is None


def test_malformed_model_json_is_rejected():
    def malformed(_request):
        return httpx.Response(
            200, json={"choices": [{"message": {"content": "not-json"}}]}
        )

    assert gateway(malformed).choose([candidate("rice")], SoftPreferences()) is None


def test_candidate_outside_filtered_set_is_rejected():
    def unknown(_request):
        content = json.dumps({"candidate_id": "unsafe", "reason": "guess"})
        return httpx.Response(200, json={"choices": [{"message": {"content": content}}]})

    assert gateway(unknown).choose([candidate("rice")], SoftPreferences()) is None


def test_successful_structured_selection_returns_candidate_and_reason():
    def selected(request):
        assert request.headers["authorization"] == "Bearer secret-value"
        content = json.dumps({"candidate_id": "rice", "reason": "符合偏好"})
        return httpx.Response(200, json={"choices": [{"message": {"content": content}}]})

    choice = gateway(selected).choose([candidate("rice")], SoftPreferences())
    assert choice.candidate_id == "rice"
    assert choice.reason == "符合偏好"
