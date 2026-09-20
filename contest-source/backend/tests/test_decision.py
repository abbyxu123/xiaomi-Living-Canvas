from living_canvas_backend.decision import load_catalog, recommend
from living_canvas_backend.schema import Candidate, HardConstraints, SoftPreferences


def candidate(candidate_id, **overrides):
    values = {
        "candidate_id": candidate_id,
        "name": candidate_id,
        "cuisine": "中式",
        "temperature": "热",
        "price_cny": 32,
        "delivery_minutes": 25,
        "channels": ["delivery"],
        "ingredients": ["米饭"],
        "allergens": [],
        "tags": ["暖胃"],
        "novelty_score": 0.5,
    }
    values.update(overrides)
    return Candidate(**values)


def test_recommendation_never_returns_hard_constraint_violation():
    result = recommend(
        [
            candidate("cheap", price_cny=28),
            candidate("favorite-but-expensive", cuisine="川菜", price_cny=88),
        ],
        HardConstraints(max_total_price_cny=50),
        SoftPreferences(cuisines=["川菜"]),
    )
    assert result.candidate.candidate_id == "cheap"


def test_prefers_requested_cuisine_and_temperature():
    result = recommend(
        [
            candidate("cold-western", cuisine="西式", temperature="冷"),
            candidate("hot-sichuan", cuisine="川菜", temperature="热"),
        ],
        HardConstraints(),
        SoftPreferences(cuisines=["川菜"], temperatures=["热"]),
    )
    assert result.candidate.candidate_id == "hot-sichuan"


def test_novelty_mode_changes_ranking():
    choices = [
        candidate("familiar", novelty_score=0.1),
        candidate("balanced", novelty_score=0.5),
        candidate("exploratory", novelty_score=0.9),
    ]
    assert recommend(
        choices, HardConstraints(), SoftPreferences(novelty="balanced")
    ).candidate.candidate_id == "balanced"
    assert recommend(
        choices, HardConstraints(), SoftPreferences(novelty="exploratory")
    ).candidate.candidate_id == "exploratory"


def test_rules_only_recommendation_is_marked_as_fallback():
    result = recommend(
        [candidate("warm-rice")], HardConstraints(), SoftPreferences()
    )
    assert result.source == "rules"
    assert result.rules_fallback is True


def test_allowed_model_choice_is_used_without_rules_fallback():
    result = recommend(
        [candidate("model-choice"), candidate("other")],
        HardConstraints(),
        SoftPreferences(),
        model_candidate_id="model-choice",
    )
    assert result.candidate.candidate_id == "model-choice"
    assert result.source == "model"
    assert result.rules_fallback is False


def test_returns_none_when_every_candidate_is_rejected():
    result = recommend(
        [candidate("slow", delivery_minutes=70)],
        HardConstraints(max_delivery_minutes=30),
        SoftPreferences(),
    )
    assert result is None


def test_owned_catalog_contains_demo_food_directions():
    catalog = load_catalog()
    assert len(catalog) >= 5
    assert all(item.search_query for item in catalog)
