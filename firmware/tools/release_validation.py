"""Validate built binaries independently of the firmware's runtime checks."""
import hashlib
import struct
import zlib

FLASH_SIZE = 0x400000
APP_SIZE = 0x1E0000
EXPECTED_PARTITIONS = {
    "nvs": (1, 2, 0x9000, 0x5000),
    "otadata": (1, 0, 0xE000, 0x2000),
    "ota_0": (0, 0x10, 0x10000, APP_SIZE),
    "ota_1": (0, 0x11, 0x1F0000, APP_SIZE),
    "spiffs": (1, 0x82, 0x3D0000, 0x30000),
}

def validate_partitions(data):
    found = {}
    ends = []
    md5_seen = False
    for offset in range(0, len(data), 32):
        entry = data[offset:offset + 32]
        if len(entry) != 32:
            raise ValueError("Onvolledige partitietabel")
        magic = entry[:2]
        if magic == b"\xff\xff":
            break
        if magic == b"\xeb\xeb":
            if entry[16:] != hashlib.md5(data[:offset]).digest():
                raise ValueError("Partitie-MD5 klopt niet")
            md5_seen = True
            continue
        if magic != b"\xaa\x50":
            raise ValueError("Ongeldige partitie-entry")
        _, kind, subtype, address, size, label, flags = struct.unpack("<HBBII16sI", entry)
        name = label.split(b"\0", 1)[0].decode()
        if name in found or flags:
            raise ValueError("Dubbele of onverwacht versleutelde partitie")
        if not size or address + size > FLASH_SIZE:
            raise ValueError("Partitie past niet in 4 MB flash")
        if any(address < end and address + size > start for start, end in ends):
            raise ValueError("Partities overlappen")
        ends.append((address, address + size))
        found[name] = (kind, subtype, address, size)
    if not md5_seen or found != EXPECTED_PARTITIONS:
        raise ValueError("Partitietabel wijkt af van de geteste 4 MB OTA-indeling")
    return found

def validate_image(data, project=None, version=None):
    if len(data) < 32 or data[0] != 0xE9 or not 1 <= data[1] <= 16:
        raise ValueError("Geen ESP32-image")
    if struct.unpack_from("<H", data, 12)[0] != 0:
        raise ValueError("Image is voor een andere ESP-chip")
    if data[2] != 2 or data[3] != 0x20:
        raise ValueError("Verwacht DIO / 40 MHz / 4 MB-header")
    checksum = 0xEF
    position = 24
    for _ in range(data[1]):
        if position + 8 > len(data):
            raise ValueError("Afgekapt segmentheader")
        _, size = struct.unpack_from("<II", data, position)
        position += 8
        if position + size > len(data):
            raise ValueError("Afgekapt imagesegment")
        for byte in data[position:position + size]:
            checksum ^= byte
        position += size
    checksum_offset = position + (15 - position % 16)
    if checksum_offset >= len(data) or data[checksum_offset] != checksum:
        raise ValueError("ESP-imagechecksum klopt niet")
    payload_end = checksum_offset + 1
    if data[23] != 1 or len(data) != payload_end + 32:
        raise ValueError("Ontbrekende SHA-256 of onverwachte bytes achter image")
    if hashlib.sha256(data[:payload_end]).digest() != data[payload_end:]:
        raise ValueError("ESP-image SHA-256 klopt niet")
    if project is not None:
        if len(data) > APP_SIZE or data[32:36] != b"\x32\x54\xcd\xab":
            raise ValueError("Geen passende applicatie-image")
        if data[80:112].split(b"\0", 1)[0].decode() != project:
            raise ValueError("Verkeerde projectnaam")
        if data[48:80].split(b"\0", 1)[0].decode() != version:
            raise ValueError("Verkeerde firmwareversie")
    return {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}

def validate_factory(factory, components):
    for address, data in components:
        if address + len(data) > FLASH_SIZE or factory[address:address + len(data)] != data:
            raise ValueError(f"Component niet correct op flashadres {address:#x}")
    if len(factory) != max(address + len(data) for address, data in components):
        raise ValueError("Onverwachte grootte factory-image")
    if factory[:0x1000] != b"\xff" * 0x1000:
        raise ValueError("Bootloader begint niet op 0x1000")
    if factory[0x9000:0xE000] != b"\xff" * 0x5000:
        raise ValueError("Factory-image bevat onverwachte configuratiegegevens")
    validate_ota_start(factory[0xE000:0x10000])

def validate_ota_start(data):
    # SDK boot_app0: sequence 1 chooses (sequence - 1) % 2 = ota_0.
    if len(data) != 0x2000 or struct.unpack_from("<I", data, 0)[0] != 1:
        raise ValueError("OTA-startdata selecteert niet ota_0")
    if struct.unpack_from("<I", data, 24)[0] != 0xFFFFFFFF:
        raise ValueError("Onverwachte initiële OTA-status")
    if struct.unpack_from("<I", data, 28)[0] != zlib.crc32(data[:4], 0xFFFFFFFF):
        raise ValueError("OTA-startdata CRC klopt niet")
