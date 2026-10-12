"""Compile and execute the production protocol code on the development computer."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="ctek-host-tests-") as folder:
    for suite in ("protocol", "session", "ble_diagnostics", "image", "telemetry", "admin"):
        executable = Path(folder) / (suite + "_test")
        subprocess.run([
            os.environ.get("CXX", "g++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
            "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g",
            "-I", str(root / "include"), str(root / "src/ctek_protocol.cpp"),
            str(root / "src/ctek_session.cpp"), str(root / f"test/{suite}_test.cpp"),
            "-o", str(executable),
        ], check=True)
        subprocess.run([str(executable)], check=True)
