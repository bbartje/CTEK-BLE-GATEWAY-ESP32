"""Stamp Arduino's generic app descriptor before esptool creates the image.

Only the linked ELF output is edited. SDK and sources remain intact; esptool
subsequently calculates the application checksum and SHA from the final ELF.
"""
from pathlib import Path
import re
from elftools.elf.elffile import ELFFile

Import("env")

def stamp_descriptor(source, target, env):
    path = Path(str(target[0]))
    with path.open("rb") as stream:
        elf = ELFFile(stream)
        symbols = elf.get_section_by_name(".symtab").get_symbol_by_name("esp_app_desc")
        if not symbols:
            raise RuntimeError("ESP app descriptor ontbreekt in ELF")
        symbol = symbols[0]
        section = elf.get_section(symbol["st_shndx"])
        offset = section["sh_offset"] + symbol["st_value"] - section["sh_addr"]
    header = (Path(env["PROJECT_DIR"]) / "include/gateway_types.h").read_text()
    version = re.search(r'kFirmwareVersion\[\] = "([^"]+)"', header).group(1).encode()
    if len(version) >= 32:
        raise RuntimeError("Firmwareversie te lang voor ESP-header")
    with path.open("r+b") as stream:
        stream.seek(offset)
        if stream.read(4) != bytes.fromhex("3254cdab"):
            raise RuntimeError("Ongeldige ESP app descriptor")
        stream.seek(offset + 16)
        stream.write(version.ljust(32, b"\0"))
        stream.write(b"ctek-ble-gateway".ljust(32, b"\0"))
    print("CTEK-appbeschrijving:", version.decode(), "ctek-ble-gateway")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", stamp_descriptor)
