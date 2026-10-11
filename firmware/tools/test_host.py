"""Compile and execute the production protocol code on the development computer."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="ctek-host-tests-") as folder:
    executable = Path(folder) / "protocol_test"
    subprocess.run([
        os.environ.get("CXX", "g++"), "-std=c++17", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g",
        "-I", str(root / "include"), str(root / "src/ctek_protocol.cpp"),
        str(root / "test/protocol_test.cpp"), "-o", str(executable),
    ], check=True)
    subprocess.run([str(executable)], check=True)
