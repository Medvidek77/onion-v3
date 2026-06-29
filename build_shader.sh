#!/bin/sh
# Build optimized shader.spv with RDNA2 VGPR optimization
# Requires: glslangValidator, spirv-dis, spirv-as (from spirv-tools or vulkan-sdk)

set -e
SHADER="shader.comp"
OUT="shader.spv"
TMP_DIS="/tmp/_shader_dis.txt"
TMP_PATCHED="/tmp/_shader_patched.txt"

echo "[1/3] Compiling GLSL..."
glslangValidator -V "$SHADER" -o "$OUT"

echo "[2/3] Applying DontInline to fe_sq + fe_mul..."
spirv-dis "$OUT" -o "$TMP_DIS"
sed 's/%fe_mul_i1_10__i1_10__i1_10__ = OpFunction %void None /%fe_mul_i1_10__i1_10__i1_10__ = OpFunction %void DontInline /' \
    "$TMP_DIS" > "$TMP_PATCHED"
sed 's/%fe_sq_i1_10__i1_10__ = OpFunction %void None /%fe_sq_i1_10__i1_10__ = OpFunction %void DontInline /' \
    "$TMP_PATCHED" > "${TMP_PATCHED}.2"
mv "${TMP_PATCHED}.2" "$TMP_PATCHED"
spirv-as "$TMP_PATCHED" -o "$OUT"

echo "[3/3] Done: $OUT ($(wc -c < $OUT) bytes)"
echo "Info: SPIR-V patched with DontInline on fe_sq and fe_mul to reduce VGPR pressure."
