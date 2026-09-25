from pathlib import Path

import tomli


def test_runtime_resources_are_declared_as_package_data():
    pyproject = Path(__file__).parents[1] / "pyproject.toml"
    config = tomli.loads(pyproject.read_text(encoding="utf-8"))

    package_data = config["tool"]["setuptools"]["package-data"]
    assert package_data["living_canvas_backend"] == [
        "catalog.json",
        "static/*.html",
    ]
