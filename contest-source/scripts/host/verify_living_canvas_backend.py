#!/usr/bin/env python3
"""Verify the Living Canvas P0 flow against its independent backend.

This intentionally uses only the Python standard library so it can run from
the Mac or the openvela build VM without installing another client package.
"""

from __future__ import annotations

import argparse
import json
import sys
import urllib.error
import urllib.parse
import urllib.request
from typing import Optional


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None


def request_json(base_url: str, path: str, body: Optional[dict] = None) -> dict:
    data = None
    headers = {"Accept": "application/json"}
    method = "GET"
    if body is not None:
        data = json.dumps(body, ensure_ascii=False).encode("utf-8")
        headers["Content-Type"] = "application/json"
        method = "POST"

    request = urllib.request.Request(
        urllib.parse.urljoin(base_url + "/", path.lstrip("/")),
        data=data,
        headers=headers,
        method=method,
    )
    try:
        with urllib.request.urlopen(request, timeout=15) as response:
            return json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as error:
        detail = error.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"{path} returned HTTP {error.code}: {detail}") from error
    except urllib.error.URLError as error:
        raise RuntimeError(
            f"cannot reach Living Canvas backend at {base_url}: {error.reason}"
        ) from error


def verify_redirect(base_url: str, token: str, session_id: str) -> None:
    opener = urllib.request.build_opener(NoRedirect)
    request = urllib.request.Request(
        urllib.parse.urljoin(base_url + "/", f"c/{token}"), method="GET"
    )
    try:
        opener.open(request, timeout=15)
    except urllib.error.HTTPError as error:
        if error.code != 307:
            raise RuntimeError(f"compact handoff returned HTTP {error.code}") from error
        expected = f"/console?session={session_id}"
        actual = error.headers.get("Location", "")
        if actual != expected:
            raise RuntimeError(
                f"compact handoff location mismatch: expected {expected}, got {actual}"
            )
        return
    raise RuntimeError("compact handoff did not return HTTP 307")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--base-url",
        default="http://127.0.0.1:8765",
        help="running Living Canvas backend URL",
    )
    args = parser.parse_args()
    base_url = args.base_url.rstrip("/")

    health = request_json(base_url, "/health")
    require(
        health.get("service") == "living-canvas-decision-backend"
        and health.get("status") == "ok",
        "Living Canvas health check is not ok",
    )

    created = request_json(base_url, "/v1/session", {"device_id": "gemini-s1"})
    session_id = str(created.get("session_id", ""))
    require(session_id.startswith("meal_") and len(session_id) == 17,
            "backend returned an invalid session id")

    decision = request_json(
        base_url,
        "/v1/input",
        {
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
        },
    )
    require(decision.get("state") == "candidate",
            f"decision did not reach candidate state: {decision.get('state')}")
    recommendation = decision.get("recommendation") or {}
    require(bool(recommendation.get("candidate")), "decision has no candidate")

    event = request_json(
        base_url,
        "/v1/device/event",
        {
            "session_id": session_id,
            "event": "accept_candidate",
        },
    )
    require(event.get("state") == "confirming",
            f"device confirmation failed: {event.get('state')}")

    confirmed = request_json(
        base_url,
        "/v1/confirm",
        {"session_id": session_id, "platform": "eleme"},
    )
    require(confirmed.get("state") == "handed_off", "backend confirmation failed")
    require(
        str(confirmed.get("external_url", "")).startswith("https://"),
        "backend did not return a phone-safe web URL",
    )
    require(
        confirmed.get("requires_user_payment") is True,
        "handoff must keep final payment with the user",
    )

    token = session_id.removeprefix("meal_")
    verify_redirect(base_url, token, session_id)

    chosen_id = recommendation["candidate"].get("candidate_id", "")
    print(
        f"PASS: Living Canvas takeout flow session={session_id} "
        f"candidate={chosen_id}"
    )
    print(
        f"PASS: compact handoff {base_url}/c/{token} "
        f"-> /console?session={session_id}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
