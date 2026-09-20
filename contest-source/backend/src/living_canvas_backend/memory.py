import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from typing import List


class MemoryStore:
    def __init__(self, data_dir: Path):
        self.database_path = Path(data_dir) / "living_canvas_memory.sqlite3"

    def _connect(self):
        self.database_path.parent.mkdir(parents=True, exist_ok=True)
        connection = sqlite3.connect(str(self.database_path))
        connection.row_factory = sqlite3.Row
        connection.executescript(
            """
            CREATE TABLE IF NOT EXISTS feedback (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                session_id TEXT NOT NULL,
                candidate_id TEXT NOT NULL,
                liked INTEGER NOT NULL,
                note TEXT,
                created_at TEXT NOT NULL
            );
            CREATE TABLE IF NOT EXISTS remembered_preferences (
                value TEXT PRIMARY KEY,
                created_at TEXT NOT NULL
            );
            """
        )
        return connection

    def add_feedback(
        self,
        session_id: str,
        candidate_id: str,
        liked: bool,
        note: str,
        remember_preferences: List[str],
    ):
        created_at = datetime.now(timezone.utc).isoformat()
        with self._connect() as connection:
            cursor = connection.execute(
                "INSERT INTO feedback(session_id, candidate_id, liked, note, created_at) "
                "VALUES (?, ?, ?, ?, ?)",
                (session_id, candidate_id, int(liked), note or None, created_at),
            )
            for preference in remember_preferences:
                value = preference.strip()
                if value:
                    connection.execute(
                        "INSERT OR IGNORE INTO remembered_preferences(value, created_at) "
                        "VALUES (?, ?)",
                        (value, created_at),
                    )
            feedback_id = cursor.lastrowid
        return {
            "feedback_id": feedback_id,
            "session_id": session_id,
            "candidate_id": candidate_id,
            "liked": liked,
            "note": note or None,
        }

    def list_all(self):
        with self._connect() as connection:
            feedback_rows = connection.execute(
                "SELECT session_id, candidate_id, liked, note, created_at "
                "FROM feedback ORDER BY id DESC"
            ).fetchall()
            preference_rows = connection.execute(
                "SELECT value FROM remembered_preferences ORDER BY created_at, value"
            ).fetchall()
        return {
            "feedback": [
                {
                    "session_id": row["session_id"],
                    "candidate_id": row["candidate_id"],
                    "liked": bool(row["liked"]),
                    "note": row["note"],
                    "created_at": row["created_at"],
                }
                for row in feedback_rows
            ],
            "remembered_preferences": [row["value"] for row in preference_rows],
        }

    def delete_all(self):
        with self._connect() as connection:
            connection.execute("DELETE FROM feedback")
            connection.execute("DELETE FROM remembered_preferences")
