import hashlib, base64, glob

folders = glob.glob('keys/*')
for f in folders:
    try:
        with open(f + '/hs_ed25519_secret_key', 'rb') as file:
            p = file.read()[-32:]
            chk = hashlib.sha3_256(b'.onion checksum' + p + b'\x03').digest()[:2]
            addr = base64.b32encode(p + chk + b'\x03').decode().lower()
            print(f'{f}: {addr}.onion')
    except Exception as e:
        print(f'Chyba u {f}: {e}')
