WASMSDK = $(shell pwd)/wasi-sdk-12.0
WASILIBC = $(shell pwd)/wasi-libc/sysroot
COMPILE = $(WASMSDK)/bin/clang --sysroot=$(WASILIBC) -Wall -O2 -g -DPy_BUILD_CORE -I$(shell pwd)/Include

OBJS = Objects/abstract.o Objects/accu.o Objects/boolobject.o Objects/bytearrayobject.o Objects/bytes_methods.o Objects/bytesobject.o Objects/capsule.o Objects/cellobject.o Objects/classobject.o Objects/codeobject.o Objects/complexobject.o Objects/descrobject.o Objects/dictobject.o Objects/enumobject.o Objects/exceptions.o Objects/fileobject.o Objects/floatobject.o Objects/frameobject.o Objects/funcobject.o Objects/genobject.o Objects/iterobject.o Objects/listobject.o Objects/longobject.o Objects/memoryobject.o Objects/methodobject.o Objects/moduleobject.o Objects/namespaceobject.o Objects/object.o Objects/obmalloc.o Objects/odictobject.o Objects/rangeobject.o Objects/setobject.o Objects/sliceobject.o Objects/structseq.o Objects/tupleobject.o Objects/typeobject.o Objects/unicodectype.o Objects/unicodeobject.o Objects/weakrefobject.o \
	Python/fileutils.o Python/errors.o Python/pystate.o Python/ceval.o Modules/gcmodule.o Python/pylifecycle.o Python/getargs.o Python/modsupport.o Python/codecs.o Python/_warnings.o Python/pyctype.o Python/pystrhex.o Python/mysnprintf.o Python/import.o Python/sysmodule.o Python/traceback.o Python/sigcheck.o Python/formatter_unicode.o Python/pyhash.o Python/compile.o Python/bltinmodule.o Python/pystrtod.o Python/structmember.o Python/pythonrun.o Python/dtoa.o Python/pytime.o

py.wasm: interp-py.c $(OBJS)
	$(COMPILE) -o $@ $^

%.o: %.c
	$(COMPILE) -c -o $@ $<
