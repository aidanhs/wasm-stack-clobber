#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
set -o xtrace

WASMSDK=$(pwd)/wasi-sdk-12.0

#cd wasi-libc
#    make -j8 WASM_CC=$WASMSDK/bin/clang WASM_AR=$WASMSDK/bin/ar WASM_NM=$WASMSDK/bin/nm
#    cd ..
WASILIBC=$(pwd)/wasi-libc/sysroot

$WASMSDK/bin/clang --sysroot=$WASILIBC -Wall -O2 -g -o py.wasm interp-py.c extra.c #-DCUSTOM_LIBC
./wasmtime-v0.22.1-x86_64-linux/wasmtime py.wasm
