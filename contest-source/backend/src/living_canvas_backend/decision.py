import json
from pathlib import Path
from typing import Iterable, List, Optional

from .constraints import filter_candidates
from .schema import Candidate, HardConstraints, Recommendation, SoftPreferences


def load_catalog(path: Optional[Path] = None) -> List[Candidate]:
    catalog_path = path or Path(__file__).with_name("catalog.json")
    with catalog_path.open("r", encoding="utf-8") as handle:
        return [Candidate.model_validate(item) for item in json.load(handle)]


def _contains(value: str, choices: Iterable[str]) -> bool:
    normalized = value.strip().casefold()
    return any(normalized == choice.strip().casefold() for choice in choices)


def _score(candidate: Candidate, preferences: SoftPreferences) -> float:
    score = 0.0
    if _contains(candidate.cuisine, preferences.cuisines):
        score += 4.0
    if _contains(candidate.temperature, preferences.temperatures):
        score += 2.0

    preferred_tags = {tag.strip().casefold() for tag in preferences.preferred_tags}
    score += sum(
        1.0 for tag in candidate.tags if tag.strip().casefold() in preferred_tags
    )

    novelty_targets = {"familiar": 0.1, "balanced": 0.5, "exploratory": 0.9}
    target = novelty_targets[preferences.novelty]
    score += 2.0 * (1.0 - abs(candidate.novelty_score - target))
    return round(score, 4)


def recommend(
    candidates: List[Candidate],
    constraints: HardConstraints,
    preferences: SoftPreferences,
    model_candidate_id: Optional[str] = None,
) -> Optional[Recommendation]:
    allowed, _ = filter_candidates(candidates, constraints)
    if not allowed:
        return None

    allowed_by_id = {item.candidate_id: item for item in allowed}
    if model_candidate_id and model_candidate_id in allowed_by_id:
        selected = allowed_by_id[model_candidate_id]
        return Recommendation(
            candidate=selected,
            reason="模型建议已通过画间硬约束复核",
            source="model",
            rules_fallback=False,
            score=_score(selected, preferences),
        )

    ranked = sorted(
        ((_score(item, preferences), item) for item in allowed),
        key=lambda pair: (-pair[0], pair[1].candidate_id),
    )
    score, selected = ranked[0]
    return Recommendation(
        candidate=selected,
        reason="符合当前预算、时效与口味偏好",
        source="rules",
        rules_fallback=True,
        score=score,
    )
