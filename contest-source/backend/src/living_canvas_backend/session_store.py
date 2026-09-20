from collections import OrderedDict
from secrets import token_hex
from typing import Optional

from .schema import DecisionSession


class SessionStore:
    def __init__(self, max_sessions: int = 200):
        self.max_sessions = max_sessions
        self._sessions = OrderedDict()

    def create(self, device_id: Optional[str] = None) -> DecisionSession:
        session_id = "meal_" + token_hex(6)
        metadata = {"device_id": device_id} if device_id else {}
        session = DecisionSession(session_id=session_id, metadata=metadata)
        self._sessions[session_id] = session
        while len(self._sessions) > self.max_sessions:
            self._sessions.popitem(last=False)
        return session

    def get(self, session_id: str) -> Optional[DecisionSession]:
        return self._sessions.get(session_id)

    def save(self, session: DecisionSession) -> None:
        self._sessions[session.session_id] = session
        self._sessions.move_to_end(session.session_id)
