#!/usr/bin/env python3
"""Converts webui/dist/index.html to a gzipped C header file for PROGMEM."""

import gzip
import os
import sys

DIST_FILE = os.path.join(os.path.dirname(__file__), '..', 'dist', 'index.html')
OUTPUT_FILE = os.path.join(os.path.dirname(__file__), '..', '..', 'marquee', 'webapp.h')


def main():
    if not os.path.exists(DIST_FILE):
        print(f"Error: {DIST_FILE} not found. Run 'npm run build' first.")
        sys.exit(1)

    with open(DIST_FILE, 'rb') as f:
        raw = f.read()

    compressed = gzip.compress(raw, compresslevel=9)

    raw_size = len(raw)
    gz_size = len(compressed)
    ratio = (1 - gz_size / raw_size) * 100 if raw_size > 0 else 0

    hex_array = ', '.join(f'0x{b:02x}' for b in compressed)

    header = f"""// AUTO-GENERATED - DO NOT EDIT
// Source: webui/dist/index.html ({raw_size} bytes -> {gz_size} bytes gzipped, {ratio:.0f}% reduction)
#pragma once
#include <Arduino.h>

const uint8_t WEBAPP_HTML_GZ[] PROGMEM = {{
  {hex_array}
}};
const size_t WEBAPP_HTML_GZ_LEN = {gz_size};
"""

    with open(OUTPUT_FILE, 'w') as f:
        f.write(header)

    print(f"webapp.h: {raw_size} -> {gz_size} bytes ({ratio:.0f}% compression)")


if __name__ == '__main__':
    main()
