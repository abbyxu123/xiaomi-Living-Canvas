# Living Canvas Competition Flow Implementation Plan

**Goal:** Build and verify the smallest Gemini S1 takeout flow from Living Canvas choice screen to a Living Canvas phone-handoff QR code.

**Architecture:** Add a pure-C, transport-independent controller to the existing Living Canvas application, then connect it to the owned backend contract and LVGL display. Keep all backend and board visuals in the Living Canvas project. Hardware network and button adapters are isolated so the core flow can be tested before UART-assisted device validation.

**Tech Stack:** C11, openvela/NuttX, LVGL, Living Canvas FastAPI backend, MiMo through an OpenAI-compatible API, host `make` tests.

---

### Task 1: Lock the competition state contract

**Files:**
- Create: `app/hello_app/include/lc_competition.h`
- Create: `app/hello_app/src/lc_competition.c`
- Create: `app/hello_app/tests/host/test_lc_competition.c`
- Modify: `app/hello_app/tests/host/Makefile`

**Step 1: Write the failing test**

Cover the takeout path `choice -> requesting -> recommendation -> confirm -> QR`, plus cancel, gateway error, invalid result, and rules-fallback labeling.

**Step 2: Run test to verify it fails**

Run: `make test_lc_competition`

Expected: FAIL because `lc_competition` does not exist.

**Step 3: Write minimal implementation**

Implement bounded state and result structs with no heap allocation and no network calls.

**Step 4: Run test to verify it passes**

Run: `make test_lc_competition && ./test_lc_competition`

Expected: `PASS: lc_competition`.

**Step 5: Commit**

```bash
git add app/hello_app/include/lc_competition.h app/hello_app/src/lc_competition.c app/hello_app/tests/host/test_lc_competition.c app/hello_app/tests/host/Makefile
git commit -m "feat: add competition takeout controller"
```

### Task 2: Complete the Living Canvas backend request boundary

**Files:**
- Modify: `app/hello_app/include/lc_backend_contract.h`
- Modify: `app/hello_app/src/lc_backend_contract.c`
- Modify: `app/hello_app/tests/host/test_lc_backend_contract.c`

**Step 1: Write the failing test**

Require bounded JSON for session creation, input, and final confirmation; reject invalid session IDs and truncated output.

**Step 2: Run test to verify it fails**

Run: `make test_lc_backend_contract && ./test_lc_backend_contract`

Expected: FAIL because session and confirmation builders are missing.

**Step 3: Write minimal implementation**

Add only the payload builders used by the P0 takeout flow.

**Step 4: Run test to verify it passes**

Run: `make test_lc_backend_contract && ./test_lc_backend_contract`

Expected: `PASS: lc_backend_contract`.

**Step 5: Commit**

```bash
git add app/hello_app/include/lc_backend_contract.h app/hello_app/src/lc_backend_contract.c app/hello_app/tests/host/test_lc_backend_contract.c
git commit -m "feat: complete Living Canvas handoff contract"
```

### Task 3: Add the Living Canvas competition renderer

**Files:**
- Modify: `app/hello_app/include/lc_display.h`
- Modify: `app/hello_app/src/lc_display.c`
- Modify: `app/hello_app/hello_app_main.c`
- Modify: `app/hello_app/tests/host/test_lc_main.c`
- Modify: `app/hello_app/tests/host/test_lc_ui_build.sh`
- Modify: `app/hello_app/CMakeLists.txt`
- Modify: `app/hello_app/Makefile`

**Step 1: Write the failing tests**

Require a `--competition-demo` entry point and renderer references for choice,
thinking, recommendation, confirmation, QR, and recoverable error states.

**Step 2: Run tests to verify they fail**

Run: `make test_lc_main && ./test_lc_main && sh test_lc_ui_build.sh`

Expected: FAIL because the entry point and state renderer are missing.

**Step 3: Write minimal implementation**

Reuse the current background and three icons. Render labels and panels with
LVGL primitives. Do not add expression PNGs, speech bubbles, or video.

**Step 4: Run tests to verify they pass**

Run: `make test_lc_main && ./test_lc_main && sh test_lc_ui_build.sh`

Expected: both checks pass.

**Step 5: Commit**

```bash
git add app/hello_app
git commit -m "feat: render Living Canvas competition flow"
```

### Task 4: Verify Living Canvas backend modes on the Mac

**Files:**
- Create: `scripts/host/verify_living_canvas_backend.py`
- Create: `tests/evidence/backend/.gitkeep`
- Modify: `docs/BUILD_AND_FLASH.md`

**Step 1: Write a failing gateway smoke check**

Check health, session creation, takeout input, second confirmation, compact QR
redirect, and rules-only completion against the in-repository backend.

**Step 2: Run it against a stopped gateway**

Run: `./scripts/host/verify_living_canvas_backend.py`

Expected: FAIL with an actionable gateway-unreachable message.

**Step 3: Start the Living Canvas backend with rules fallback**

Start `backend/src/living_canvas_backend` from this repository. Keep secrets
outside the contest repository.

**Step 4: Run the smoke check**

Expected: PASS and a saved redacted evidence file.

**Step 5: Repeat with MiMo**

Set `MODEL_BASE_URL`, `MODEL_NAME`, and `MODEL_API_KEY`; never print or commit
the key. Record only model name, pass/fail, and latency.

### Task 5: Build the candidate firmware

**Files:**
- Modify: `tests/evidence/build/`
- Modify: `docs/BUILD_AND_FLASH.md`

**Step 1: Run the entire host suite**

Run: `make clean && make test` from `app/hello_app/tests/host`.

Expected: all C tests pass.

**Step 2: Sync the contest overlay into the existing Ubuntu openvela workspace**

Do not replace the pinned vendor baseline or flash the board.

**Step 3: Build**

Run the pinned Gemini S1 `nsh_minidisplay` build command already recorded in
`BUILD_EVIDENCE.md`.

Expected: exit code 0 and a linked `nsh.fex` below the observed partition size.

**Step 4: Hash and back up the candidate**

Record SHA-256, byte size, build command, and `NOT_FLASHED` status.

### Task 6: UART-assisted physical validation

**Files:**
- Modify: `tests/evidence/device/`
- Modify: `docs/BUILD_AND_FLASH.md`

**Step 1: Capture the factory boot log before flashing**

Identify boot stage, storage, recovery input, and serial parameters.

**Step 2: Confirm the documented recovery route**

Do not flash until the board can be returned to the backed-up factory image.

**Step 3: Flash the hashed candidate once**

Use the vendor-supported path confirmed from the UART log.

**Step 4: Validate the physical story**

Verify display orientation/color, buttons, backend connectivity, MiMo result,
rules fallback, second confirmation, QR scan, cancel, and reboot recovery.

**Step 5: Record evidence**

Save redacted logs, photos/video timestamps, measured latency, and the exact
firmware hash for the submission report.
