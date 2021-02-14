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
$WASMSDK/bin/clang --sysroot=$WASILIBC -Wl,--allow-undefined -Wall -O2 -g -DPy_BUILD_CORE -IInclude -o ../dist/py.wasm interp-py.c obmalloc.c fileutils.c unicodeobject.c -Wl,--export-all
