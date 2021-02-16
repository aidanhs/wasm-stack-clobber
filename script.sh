#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
set -o xtrace

WASMSDK=$(pwd)/wasi-sdk-12.0

#$WASMSDK/bin/clang --sysroot=$WASMSDK/share/wasi-sysroot -Wl,--allow-undefined -Wall -O2 -g -DPy_BUILD_CORE -IInclude -o ../dist/py.wasm interp-py.c obmalloc.c fileutils.c unicodeobject.c -Wl,--export-all

##cd wasi-libc
##    make -j8 WASM_CC=$WASMSDK/bin/clang WASM_AR=$WASMSDK/bin/ar WASM_NM=$WASMSDK/bin/nm
##    cd ..
WASILIBC=$(pwd)/wasi-libc/sysroot
#$WASMSDK/bin/clang --sysroot=$WASILIBC -Wall -O2 -g -DPy_BUILD_CORE -IInclude -o ../dist/py.wasm interp-py.c obmalloc.c fileutils.c unicodeobject.c exceptions.c object.c bytesobject.c -Wl,-error-limit=0 #-Wl,--allow-undefined
# pyctype.c pystate.c
$WASMSDK/bin/clang --sysroot=$WASILIBC -Wall -O2 -g -DPy_BUILD_CORE -IInclude -o py.wasm interp-py.c ../cpython/libpython3.5.a
./wasmtime-v0.22.1-x86_64-linux/wasmtime py.wasm
