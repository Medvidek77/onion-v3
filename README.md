# tor_vanity_vk

Vulkan compute vanity address generator for Tor v3 hidden services.
Rewritten from OpenCL to be explicitly designed for modern AMD/Nvidia GPUs under POSIX environments like FreeBSD using `radv` or `amdvlk`.

## Dependencies

- C compiler (gcc / clang)
- Vulkan development headers, runtime, and GLSL compiler (`shaderc` or `glslang`)
- libsodium

On FreeBSD:
`pkg install gcc vulkan-headers vulkan-loader shaderc libsodium`

## Building

```
make
```
This will compile the C host application and use `glslc` to compile `shader.comp` into `shader.spv`.

## Usage

```
./tor_vanity_vk <prefix> <output_dir>
```

Example:
```
./tor_vanity_vk test mykeys/
```

This will run in batches of 1M keys per kernel execution using the Vulkan compute pipeline.
When a match is found, it will generate a tor-compatible folder inside `<output_dir>` containing `hs_ed25519_secret_key`.

You can then copy the generated key directory into your tor's `HiddenServiceDir`.
