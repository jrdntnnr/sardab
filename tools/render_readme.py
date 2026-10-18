#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Compose actual readme_shots.c framebuffer captures at an integer 2x scale."""
import argparse
from pathlib import Path
from PIL import Image

ap = argparse.ArgumentParser(description=__doc__)
ap.add_argument('captures', type=Path, help='directory containing boot/home/presets/menu.ppm')
ap.add_argument('output', type=Path, nargs='?', default=Path('docs/sardab-2.3.png'))
a = ap.parse_args()
strip = Image.new('RGB', (2000, 512), (5, 5, 15))
for i, name in enumerate(('boot', 'home', 'presets', 'menu')):
    frame = Image.open(a.captures / (name + '.ppm'))
    assert frame.size == (240, 240), (name, frame.size)
    strip.paste(frame.resize((480, 480), Image.Resampling.NEAREST), (16 + i * 496, 16))
strip.save(a.output)
print(a.output)
