import sys
import base64
import hashlib
import os

def get_onion(secret_file):
    with open(secret_file, 'rb') as f:
        data = f.read()

    # tor secret files usually have a 32-byte header followed by 64 bytes of expanded key + pubkey
    if len(data) != 96:
        print(f"Invalid secret file size: {len(data)}")
        return

    pubkey = data[64:96]

    checksum = hashlib.sha3_256(b".onion checksum" + pubkey + b"\x03").digest()[:2]

    version = b"\x03"

    onion_bytes = pubkey + checksum + version

    onion = base64.b32encode(onion_bytes).decode('ascii').lower()
    print(onion + ".onion")

if len(sys.argv) < 2:
    print("Usage: python3 genpubs.py <secret_file>")
    sys.exit(1)

get_onion(sys.argv[1])
