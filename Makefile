.PHONY: clean
.PHONY: py.wasm # not really, but always keep it dirty while rebuilding libc

WASMSDK = $(shell pwd)/wasi-sdk-12.0
WASILIBC = $(shell pwd)/wasi-libc/sysroot
COMPILE = $(WASMSDK)/bin/clang --sysroot=$(WASILIBC) -Wall -O2 -g -DPy_BUILD_CORE

py.wasm: interp-py.c extra.c
	$(COMPILE) -o $@ $^

clean:
	rm -f $(OBJS)

%.o: %.c
	$(COMPILE) -c -o $@ $<
