"""Print offline DGUSII touch frames. Never opens a serial port or connects hardware."""
import argparse

KEYS = {"release": 0, "next": 1, "previous": 2, "confirm": 3, "back": 4, "stop": 5, "finish": 6}

def crc16(data):
    value = 0xFFFF
    for byte in data:
        value ^= byte
        for _ in range(8):
            value = (value >> 1) ^ (0xA001 if value & 1 else 0)
    return value

def event_frame(token, key, crc=True):
    if not 1 <= token <= 0xFFFF or key not in KEYS.values():
        raise ValueError("A current nonzero 16-bit token and a defined key are required")
    body = b"\x83\x50\x00\x02" + token.to_bytes(2, "big") + key.to_bytes(2, "big")
    if crc:
        body += crc16(body).to_bytes(2, "little")
    return b"\x5A\xA5" + bytes([len(body)]) + body

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("key", choices=KEYS)
    parser.add_argument("--token", required=True, type=lambda value: int(value, 0))
    parser.add_argument("--no-crc", action="store_true")
    args = parser.parse_args()
    print(event_frame(args.token, KEYS[args.key], not args.no_crc).hex(" ").upper())

if __name__ == "__main__":
    main()
