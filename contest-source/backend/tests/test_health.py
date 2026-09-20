def test_health_identifies_living_canvas_backend(client):
    response = client.get("/health")

    assert response.status_code == 200
    assert response.json() == {
        "service": "living-canvas-decision-backend",
        "status": "ok",
    }
