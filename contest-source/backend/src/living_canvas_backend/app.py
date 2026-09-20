import re
import os
from pathlib import Path
from typing import Optional

from fastapi import FastAPI, HTTPException
from fastapi.responses import HTMLResponse, RedirectResponse

from .decision import load_catalog, recommend
from .constraints import filter_candidates
from .handoff import build_platform_search_url, is_allowed_url
from .memory import MemoryStore
from .model_gateway import ModelGateway
from .schema import (
    BoardInputRequest,
    ConfirmRequest,
    DeviceEventRequest,
    FeedbackRequest,
    SessionCreateRequest,
    SessionState,
)
from .session_store import SessionStore


def create_app(
    data_dir: Optional[Path] = None,
    model_gateway: Optional[ModelGateway] = None,
) -> FastAPI:
    app = FastAPI(title="Living Canvas Decision Backend", version="0.1.0")
    sessions = SessionStore(max_sessions=200)
    memory = MemoryStore(
        Path(data_dir or os.environ.get("LIVING_CANVAS_DATA_DIR", "./var"))
    )
    model_gateway = model_gateway or ModelGateway.from_env()
    catalog = load_catalog()
    app.state.sessions = sessions
    app.state.memory = memory

    @app.get("/health")
    def health():
        return {
            "service": "living-canvas-decision-backend",
            "status": "ok",
        }

    @app.post("/v1/session", status_code=201)
    def create_session(request: SessionCreateRequest):
        return sessions.create(request.device_id)

    @app.post("/v1/input")
    def submit_input(request: BoardInputRequest):
        session = sessions.get(request.session_id)
        if session is None:
            raise HTTPException(status_code=404, detail="session_not_found")

        constraints = request.hard_constraints.as_domain()
        allowed, _ = filter_candidates(catalog, constraints)
        model_choice = model_gateway.choose(allowed, request.soft_preferences)
        recommendation = recommend(
            catalog,
            constraints,
            request.soft_preferences,
            model_candidate_id=(
                model_choice.candidate_id if model_choice is not None else None
            ),
        )
        if recommendation is None:
            raise HTTPException(status_code=409, detail="no_safe_candidate")

        updated = session.model_copy(
            update={
                "state": SessionState.CANDIDATE,
                "context": request.context,
                "hard_constraints": constraints,
                "soft_preferences": request.soft_preferences,
                "recommendation": recommendation,
            }
        )
        sessions.save(updated)
        return updated

    @app.get("/v1/session/{session_id}")
    def get_session(session_id: str):
        session = sessions.get(session_id)
        if session is None:
            raise HTTPException(status_code=404, detail="session_not_found")
        return session

    @app.post("/v1/device/event")
    def device_event(request: DeviceEventRequest):
        session = sessions.get(request.session_id)
        if session is None:
            raise HTTPException(status_code=404, detail="session_not_found")
        if request.event == "accept_candidate" and session.state == SessionState.CANDIDATE:
            updated = session.model_copy(update={"state": SessionState.CONFIRMING})
        elif request.event == "cancel":
            updated = session.model_copy(update={"state": SessionState.CANCELLED})
        else:
            raise HTTPException(status_code=409, detail="invalid_state_transition")
        sessions.save(updated)
        return updated

    @app.post("/v1/confirm")
    def confirm(request: ConfirmRequest):
        session = sessions.get(request.session_id)
        if session is None:
            raise HTTPException(status_code=404, detail="session_not_found")
        if session.state != SessionState.CONFIRMING or session.recommendation is None:
            raise HTTPException(status_code=409, detail="confirmation_required")
        try:
            external_url = build_platform_search_url(
                request.platform,
                session.recommendation.candidate.search_query
                or session.recommendation.candidate.name,
            )
        except ValueError as error:
            raise HTTPException(status_code=422, detail=str(error)) from error
        if not is_allowed_url(external_url):
            raise HTTPException(status_code=500, detail="unsafe_handoff_url")

        updated = session.model_copy(
            update={
                "state": SessionState.HANDED_OFF,
                "metadata": {
                    **session.metadata,
                    "platform": request.platform,
                    "external_url": external_url,
                },
            }
        )
        sessions.save(updated)
        token = request.session_id.removeprefix("meal_")
        return {
            "session_id": request.session_id,
            "state": updated.state,
            "external_url": external_url,
            "compact_path": "/c/" + token,
            "requires_user_payment": True,
        }

    @app.get("/c/{token}")
    def compact_handoff(token: str):
        if re.fullmatch(r"[0-9a-f]{12}", token) is None:
            raise HTTPException(status_code=404, detail="handoff_not_found")
        session_id = "meal_" + token
        if sessions.get(session_id) is None:
            raise HTTPException(status_code=404, detail="handoff_not_found")
        return RedirectResponse("/console?session=" + session_id, status_code=307)

    @app.get("/console", response_class=HTMLResponse)
    def phone_console():
        html_path = Path(__file__).with_name("static") / "console.html"
        return HTMLResponse(html_path.read_text(encoding="utf-8"))

    @app.post("/v1/feedback", status_code=201)
    def save_feedback(request: FeedbackRequest):
        session = sessions.get(request.session_id)
        if session is None:
            raise HTTPException(status_code=404, detail="session_not_found")
        if session.state != SessionState.HANDED_OFF or session.recommendation is None:
            raise HTTPException(status_code=409, detail="confirmed_choice_required")
        return memory.add_feedback(
            session_id=request.session_id,
            candidate_id=session.recommendation.candidate.candidate_id,
            liked=request.liked,
            note=request.note or "",
            remember_preferences=request.remember_preferences,
        )

    @app.get("/v1/memory")
    def list_memory():
        return memory.list_all()

    @app.delete("/v1/memory")
    def delete_memory():
        memory.delete_all()
        return {"deleted": True}

    return app


app = create_app()
