from living_canvas_backend.constraints import check_candidate, filter_candidates
from living_canvas_backend.schema import Candidate, HardConstraints


def candidate(**overrides):
    values = {
        "candidate_id": "warm-rice",
        "name": "暖心盖饭",
        "cuisine": "中式",
        "temperature": "热",
        "price_cny": 32,
        "delivery_minutes": 25,
        "channels": ["delivery"],
        "ingredients": ["米饭", "鸡肉", "西兰花"],
        "allergens": [],
        "tags": ["米饭", "清淡"],
    }
    values.update(overrides)
    return Candidate(**values)


def test_rejects_allergen():
    allowed, reasons = check_candidate(
        candidate(allergens=["花生"]),
        HardConstraints(allergens=["花生"]),
    )
    assert not allowed
    assert reasons == ["allergen:花生"]


def test_rejects_diet_taboo_and_dislike():
    allowed, reasons = check_candidate(
        candidate(ingredients=["猪肉", "香菜"]),
        HardConstraints(diet_taboos=["猪肉"], dislikes=["香菜"]),
    )
    assert not allowed
    assert reasons == ["diet_taboo:猪肉", "dislike:香菜"]


def test_rejects_budget_delivery_time_and_channel():
    allowed, reasons = check_candidate(
        candidate(price_cny=66, delivery_minutes=55, channels=["pickup"]),
        HardConstraints(
            max_total_price_cny=50,
            max_delivery_minutes=30,
            channel="delivery",
        ),
    )
    assert not allowed
    assert reasons == ["over_budget", "too_slow", "wrong_channel"]


def test_empty_optional_constraints_allow_candidate():
    assert check_candidate(candidate(), HardConstraints()) == (True, [])


def test_filter_candidates_returns_only_allowed_items():
    allowed, rejected = filter_candidates(
        [candidate(), candidate(candidate_id="slow", delivery_minutes=70)],
        HardConstraints(max_delivery_minutes=30),
    )
    assert [item.candidate_id for item in allowed] == ["warm-rice"]
    assert rejected == {"slow": ["too_slow"]}
