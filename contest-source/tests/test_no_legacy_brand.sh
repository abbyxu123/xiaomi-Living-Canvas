#!/usr/bin/env bash
set -euo pipefail

denied_token="$(printf '\163\151\144')"
pattern="(^|[^[:alpha:]])${denied_token}([^[:alpha:]]|$)"

if git grep -n -i -E "$pattern" -- .; then
  echo "FAIL: legacy project branding remains in tracked files" >&2
  exit 1
fi

echo "PASS: repository contains only Living Canvas branding"
