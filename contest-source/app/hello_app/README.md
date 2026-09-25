# Living Canvas Gemini S1 application

映射到 openvela `packages/demos/contest2026_482_hello_app`。
队伍把应用代码放在本目录下。

## Manual display modes

The first firmware candidate keeps all display launches manual so a renderer
failure cannot take over startup or hide the recovery channel.

```text
living_canvas --ui-preview
living_canvas --image-preview
living_canvas --choice-preview
```

- `--ui-preview` is the original text-only display diagnostic.
- `--image-preview` shows the approved full-screen artwork without controls.
- `--choice-preview` shows the artwork plus takeout, mystery-box, and eat-at-home
  cards. Takeout is highlighted by default.

The source PNG files are verified and converted by
`tools/convert_choice_assets.py`. Device firmware contains RGB565 arrays and
does not require a PNG decoder, video decoder, SD card, or writable storage.

## Living Canvas backend boundary

`lc_backend_contract` maps the three Gemini S1 choices to the independent
Living Canvas decision backend. It only prepares bounded `/v1/session`, `/v1/input`, and
`/v1/confirm` data. Live network transport remains disabled until Gemini S1
Wi-Fi and gateway reachability are verified on hardware.

Restaurant selection and payment are never silently submitted. Real ordering
still requires an authorized provider/account and a separate user confirmation
and payment handoff.
