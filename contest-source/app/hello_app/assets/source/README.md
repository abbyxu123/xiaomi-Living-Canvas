# Approved Gemini S1 Choice Artwork

The source artwork is supplied outside the public repository and selected with
`LC_CHOICE_ASSET_SOURCE_DIR`. It is accepted only when its size and SHA-256
match `tools/convert_choice_assets.py`.

- `sitting.png`: full-screen background.
- `takeout_bag_ui.png`: primary “help me order takeout” choice.
- `mystery_box_ui.png`: random previously-liked takeout choice.
- `homedinner.png`: inspect refrigerator / eat at home choice.

The generator emits 240x320 and 320x240 RGB565 backgrounds plus three 64x64
RGB565 cards. The generated C files are self-contained; the device never reads
the desktop PNG files at runtime.
