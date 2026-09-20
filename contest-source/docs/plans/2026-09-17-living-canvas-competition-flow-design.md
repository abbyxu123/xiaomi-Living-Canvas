# Living Canvas Competition Flow Design

## Goal

Deliver the smallest honest Gemini S1 demo that completes one visible flow:
choose takeout, obtain a recommendation, confirm it, and scan a QR code to
continue on a phone.

## Product boundary

- The Living Canvas backend supplies recommendation orchestration,
  model/rules fallback, confirmation state, and the phone handoff URL.
- Gemini S1 owns the complete visual language. No legacy artwork, screen,
  or legacy ESP32 layout is shipped on the board.
- Payment is never submitted by the board. The QR code transfers the user to a
  phone where the user reviews and pays.
- The first release uses buttons and deterministic recovery. Voice and sensors
  are optional follow-up work.

## Smallest user flow

1. Show `sitting.png` with three Living Canvas choice cards.
2. Default the selection to the takeout bag.
3. Confirm `help me order takeout`.
4. Show a short thinking state.
5. Show one recommendation, price guidance, reason, and whether it came from
   the model or the local rules fallback.
6. Require a second confirmation.
7. Show a compact QR code for the Living Canvas phone console.
8. Allow cancel/back at every destructive boundary.

Mystery box and eat-at-home remain selectable contract values, but the contest
critical path is takeout first.

## Character-to-agent presentation

The system is one primary agent with shared tools and constraints. Characters
are presentation roles, not five independently authorized agents.

| Living Canvas role | Visible responsibility | Backend responsibility |
| --- | --- | --- |
| Beagle | coordinate and execute the meal task | Living Canvas orchestration and explicit confirmation |
| American shorthair | remember taste and collect feedback | preference memory and post-choice feedback |
| Pigeon | weather and lightweight reminders | optional context provider |
| Pig | ingredient and nutrition view | optional home-meal constraint provider |
| Alien | novelty and mystery-box view | optional exploration strategy |

Only the beagle and cat need to appear in the first complete demo.

## Asset budget

The first build embeds only the already converted Living Canvas background and
three 64-pixel choice icons. Character expression PNGs, speech bubbles, and MP4
animations are deferred. This avoids adding decoder dependencies and protects
the firmware partition margin.

## Architecture

The board application contains a transport-independent competition controller.
It turns user events and bounded backend results into display states. The
`lc_backend_contract` module owns request/confirmation payloads. A later hardware adapter
will carry those payloads over the verified Gemini S1 network path; until that
path is physically verified, host tests use an injected fake backend.

The Living Canvas backend remains the source of truth for recommendation and handoff. MiMo is the
primary model through the OpenAI-compatible `LIVING_CANVAS_MODEL_*` configuration.
If the model is unavailable, the backend uses deterministic rules and still completes
the flow.

## Failure behavior

- Model failure: label the result as rules fallback and continue.
- Gateway/network failure: show retry and cancel; never freeze the display.
- Missing handoff URL: stop before QR and show a recoverable error.
- Invalid or oversized backend fields: reject them before rendering.
- Board restart: return to the choice screen without submitting an action.

## Definition of done

- Host tests cover the full takeout state transition and all failure paths.
- The openvela target links successfully with the Living Canvas assets.
- The real Gemini S1 shows choice, thinking, recommendation, confirm, and QR
  screens after flashing.
- A phone scans the displayed QR and reaches the Living Canvas confirmation page.
- Both MiMo-online and rules-only modes complete the same demo.
- Build hash, latency, QR scan result, and recovery evidence are recorded for
  the submission report.
