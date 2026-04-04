# tor_vanity_ocl

OpenCL vanity address generator for Tor v3 hidden services.
Designed for OpenCL devices (e.g. AMD RX6800) under POSIX environments like FreeBSD.

## Dependencies

- C compiler (gcc / clang)
- OpenCL development headers & runtime
- libsodium

On FreeBSD:
`pkg install gcc opencl libsodium`

## Building

```
make
```

## Usage

```
./tor_vanity_ocl <prefix> <output_dir>
```

Example:
```
./tor_vanity_ocl test mykeys/
```

This will run in batches of 1M keys per kernel execution.
When a match is found, it will generate a tor-compatible folder inside `<output_dir>` containing `hs_ed25519_secret_key`.

You can then copy the generated key directory into your tor's `HiddenServiceDir`.
