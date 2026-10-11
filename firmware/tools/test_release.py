"""Check real release files and deliberately damaged copies without flashing hardware."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import zipfile

from release_validation import validate_factory, validate_image, validate_partitions, validate_ota_start

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--release", type=Path)
args = parser.parse_args()
version = re.search(r'kFirmwareVersion\[\] = "([^"]+)"', (root / "include/gateway_types.h").read_text()).group(1)
release = args.release or root / "releases" / version
manifest = json.loads((release / "manifest.json").read_text())
ota = (release / "ctek-esp32-wroom-32-ota.bin").read_bytes()
factory = (release / "ctek-esp32-wroom-32-factory.bin").read_bytes()
partitions = factory[0x8000:0x9000]

def rejected(function, *arguments):
    try:
        function(*arguments)
    except (ValueError, UnicodeError):
        return
    raise AssertionError("Een beschadigd of verkeerd firmwarebestand werd geaccepteerd")

assert manifest["version"] == version and manifest["flash_bytes"] == 0x400000
assert validate_image(ota, "ctek-ble-gateway", version)["sha256"] == manifest["files"]["ctek-esp32-wroom-32-ota.bin"]["sha256"]
assert hashlib.sha256(factory).hexdigest() == manifest["files"]["ctek-esp32-wroom-32-factory.bin"]["sha256"]
validate_partitions(partitions)
validate_ota_start(factory[0xE000:0x10000])
damaged_start = bytearray(factory[0xE000:0x10000]); damaged_start[0] = 2
rejected(validate_ota_start, damaged_start)
damaged_start = bytearray(factory[0xE000:0x10000]); damaged_start[28] ^= 1
rejected(validate_ota_start, damaged_start)
assert factory[0x10000:] == ota
assert factory[:0x1000] == b"\xff" * 0x1000 and factory[0x9000:0xE000] == b"\xff" * 0x5000
for offset in (0, 12, 80, 256, len(ota) - 1):
    damaged = bytearray(ota); damaged[offset] ^= 1
    rejected(validate_image, damaged, "ctek-ble-gateway", version)
rejected(validate_image, ota[:-100], "ctek-ble-gateway", version)
rejected(validate_image, ota, "unrelated-app", version)
rejected(validate_image, ota, "ctek-ble-gateway", "wrong-version")
rejected(validate_image, factory, "ctek-ble-gateway", version)
damaged = bytearray(partitions); damaged[8] ^= 1
rejected(validate_partitions, damaged)
# Overlapping partition with a recomputed, valid MD5: verify bounds as well as checksum.
damaged = bytearray(partitions)
struct.pack_into("<I", damaged, 3 * 32 + 4, 0x10000)
damaged[5 * 32 + 16:6 * 32] = hashlib.md5(damaged[:5 * 32]).digest()
rejected(validate_partitions, damaged)
rejected(validate_factory, factory, [(0x10001, ota)])

for line in (release / "SHA256SUMS.txt").read_text().splitlines():
    digest, name = line.split("  ", 1)
    assert hashlib.sha256((release / name).read_bytes()).hexdigest() == digest
archive = release.parent / f"ctek-esp32-wroom-32-{version}.zip"
with zipfile.ZipFile(archive) as bundle:
    assert bundle.testzip() is None
    for name in bundle.namelist(): assert bundle.read(name) == (release / name).read_bytes()

source_paths = [root / "platformio.ini", root / "partitions.csv"]
for folder in ("src", "include", "tools"):
    source_paths.extend(sorted((root / folder).glob("*.*")))
source_hash = hashlib.sha256()
for path in source_paths: source_hash.update(str(path.relative_to(root)).encode() + b"\0" + path.read_bytes())
assert source_hash.hexdigest() == manifest["firmware_source_sha256"], "Firmwarebronnen gewijzigd: bouw het flashpakket opnieuw"
print("Releasetests geslaagd: echte binaries, SHA/checksum, partities, ZIP, corrupte bestanden en bronconsistentie.")
