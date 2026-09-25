#!/usr/bin/env bash
set -euo pipefail

app_root="$(cd "$(dirname "$0")/../.." && pwd)"
skill="$app_root/skills/dinner-assistant.md"
demo="$app_root/../../docs/SKILL_DEMO.md"

test -f "$skill"
grep -q '^# Dinner Assistant$' "$skill"
grep -q '^## When to use$' "$skill"
grep -q '^## How to use$' "$skill"
grep -q '^## Example$' "$skill"
grep -q '/data/agent/skills/dinner-assistant.md' "$demo"

echo 'PASS: dinner assistant skill metadata'
