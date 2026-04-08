"""PlatformIO pre-build script: builds the Svelte webui if sources changed."""

import os
import subprocess
import sys

Import("env")

WEBUI_DIR = os.path.join(env["PROJECT_DIR"], "webui")
OUTPUT_HEADER = os.path.join(env["PROJECT_DIR"], "marquee", "webapp.h")
SRC_DIR = os.path.join(WEBUI_DIR, "src")


def get_newest_mtime(directory: str) -> float:
    newest = 0.0
    for root, dirs, files in os.walk(directory):
        dirs[:] = [d for d in dirs if d != "node_modules" and d != "dist"]
        for f in files:
            mtime = os.path.getmtime(os.path.join(root, f))
            if mtime > newest:
                newest = mtime
    return newest


def needs_rebuild() -> bool:
    if not os.path.exists(OUTPUT_HEADER):
        return True
    header_mtime = os.path.getmtime(OUTPUT_HEADER)
    src_mtime = get_newest_mtime(SRC_DIR)
    return src_mtime > header_mtime


def build_webui():
    if not needs_rebuild():
        print("WebUI: up to date, skipping build")
        return

    print("WebUI: rebuilding...")

    # Install deps if needed
    node_modules = os.path.join(WEBUI_DIR, "node_modules")
    if not os.path.exists(node_modules):
        print("WebUI: installing dependencies...")
        subprocess.check_call(["npm", "install"], cwd=WEBUI_DIR)

    # Build
    subprocess.check_call(["npm", "run", "build"], cwd=WEBUI_DIR)

    # Generate header
    gzip_script = os.path.join(WEBUI_DIR, "scripts", "gzip_to_header.py")
    subprocess.check_call([sys.executable, gzip_script])

    print("WebUI: build complete")


build_webui()
