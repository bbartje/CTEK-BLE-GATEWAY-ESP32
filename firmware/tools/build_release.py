"""Build, independently validate, and package Flash Download Tool + OTA binaries."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import zipfile

from release_validation import validate_factory, validate_image, validate_partitions
from webserver_security import guarded_parser

root = Path(__file__).resolve().parents[1]
repository = root.parent
parser = argparse.ArgumentParser()
parser.add_argument("--pio", default="pio")
parser.add_argument("--output", type=Path)
arguments = parser.parse_args()
version = re.search(r'kFirmwareVersion\[\] = "([^"]+)"', (root / "include/gateway_types.h").read_text()).group(1)
output = arguments.output or root / "releases" / version
output.mkdir(parents=True, exist_ok=True)

subprocess.run([arguments.pio, "run", "--project-dir", str(root), "-e", "esp32-wroom-32"], check=True)
subprocess.run([sys.executable, str(root / "tools/test_host.py")], check=True)
info = json.loads(subprocess.check_output([arguments.pio, "system", "info", "--json-output"], text=True))
core = Path(info["core_dir"]["value"])
python = info["python_exe"]["value"]
sdk = core / "packages/framework-arduinoespressif32"
build = root / ".pio/build/esp32-wroom-32"
esptool = core / "packages/tool-esptoolpy/esptool.py"
parser_source = sdk / "libraries/WebServer/src/Parsing.cpp"
if (build / "ctek-security/Parsing.cpp").read_text() != guarded_parser(parser_source.read_text()):
    raise RuntimeError("De beveiligde WebServer-parser ontbreekt in de build")

ota = output / "ctek-esp32-wroom-32-ota.bin"
factory = output / "ctek-esp32-wroom-32-factory.bin"
shutil.copyfile(build / "firmware.bin", ota)
components = [
    (0x1000, build / "bootloader.bin"),
    (0x8000, build / "partitions.bin"),
    (0xE000, sdk / "tools/partitions/boot_app0.bin"),
    (0x10000, ota),
]
subprocess.run([
    python, str(esptool), "--chip", "esp32", "merge_bin", "-o", str(factory),
    "--flash_mode", "dio", "--flash_freq", "40m", "--flash_size", "4MB",
    *[value for address, path in components for value in (hex(address), str(path))],
], check=True)

partitions = validate_partitions((build / "partitions.bin").read_bytes())
application = validate_image(ota.read_bytes(), "ctek-ble-gateway", version)
bootloader = validate_image((build / "bootloader.bin").read_bytes())
validate_factory(factory.read_bytes(), [(address, path.read_bytes()) for address, path in components])
# Cross-check with Espressif's parser as well as the independent checksum validation.
subprocess.run([python, str(esptool), "--chip", "esp32", "image_info", "--version", "2", str(ota)], check=True)

commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=repository, text=True).strip()
source_paths = [root / "platformio.ini", root / "partitions.csv"]
for folder in ("src", "include", "tools"):
    source_paths.extend(sorted((root / folder).glob("*.*")))
source_hash = hashlib.sha256()
for path in source_paths:
    source_hash.update(str(path.relative_to(root)).encode() + b"\0" + path.read_bytes())
manifest = {
    "version": version,
    "built_at_utc": datetime.now(timezone.utc).isoformat(),
    "base_commit": commit,
    "firmware_source_sha256": source_hash.hexdigest(),
    "chip": "ESP32-D0WD-V3 / ESP32-WROOM-32",
    "flash_bytes": 0x400000,
    "flash_mode": "DIO", "flash_frequency_mhz": 40,
    "factory_offset": "0x0", "application_offset": "0x10000",
    "partitions": {name: {"type": value[0], "subtype": value[1], "offset": hex(value[2]), "bytes": value[3]} for name, value in partitions.items()},
    "files": {ota.name: application, factory.name: {"bytes": factory.stat().st_size, "sha256": hashlib.sha256(factory.read_bytes()).hexdigest()}},
    "checks": {"host_tests": "passed", "esp32_build": "passed", "image_checksums": "passed", "flash_layout": "passed", "webserver_boundary_guard": "passed", "physical_sensor": "not tested", "mqtt_hardware": "not tested", "ota_hardware": "not tested"},
    "security_backports": ["CVE-2026-42854"],
    "dependencies": {"platformio": "6.1.19", "espressif32": "7.1.3", "arduino_esp32": "2.0.17", "nimble": "2.5.1", "pubsubclient": "2.8.0", "arduinojson": "7.4.3"},
}
(output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
for document in ("FLASHEN.md", "ACCEPTATIETEST.md", "TESTER_START.md", "TESTVERSLAG_TEMPLATE.md", "VRIJGAVE.md", f"RELEASE_{version}.md"):
    shutil.copyfile(repository / "docs" / document, output / document)
shutil.copyfile(root / "THIRD_PARTY_NOTICES.md", output / "THIRD_PARTY_NOTICES.md")
licenses = output / "licenses"; licenses.mkdir(exist_ok=True)
for library, source in [
    ("NimBLE-Arduino", root / ".pio/libdeps/esp32-wroom-32/NimBLE-Arduino/LICENSE"),
    ("Apache-NimBLE", root / ".pio/libdeps/esp32-wroom-32/NimBLE-Arduino/src/nimble/LICENSE"),
    ("tinycrypt", root / ".pio/libdeps/esp32-wroom-32/NimBLE-Arduino/src/nimble/ext/tinycrypt/LICENSE"),
    ("PubSubClient", root / ".pio/libdeps/esp32-wroom-32/PubSubClient/LICENSE.txt"),
    ("ArduinoJson", root / ".pio/libdeps/esp32-wroom-32/ArduinoJson/LICENSE.txt"),
    ("libb64", sdk / "cores/esp32/libb64/LICENSE"),
]:
    shutil.copyfile(source, licenses / (library + ".txt"))

checksums = []
for path in sorted(output.rglob("*")):
    if path.is_file() and path.name != "SHA256SUMS.txt":
        checksums.append(hashlib.sha256(path.read_bytes()).hexdigest() + "  " + str(path.relative_to(output)))
(output / "SHA256SUMS.txt").write_text("\n".join(checksums) + "\n")
archive = output.parent / f"ctek-esp32-wroom-32-{version}.zip"
with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED) as bundle:
    for path in sorted(output.rglob("*")):
        if path.is_file(): bundle.write(path, str(path.relative_to(output)))
print("Gecontroleerd flashpakket:", archive)
print("Factory:", factory, "op 0x0")
print("OTA:", ota)
