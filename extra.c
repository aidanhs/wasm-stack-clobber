#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

int
isfile(wchar_t *filename)
{
    return 0;
}

FILE *myopen(void) {
    return NULL;
}

void *myPyUnicode_DecodeUTF8(const char *string, int length, const char *errors) {
    abort();
}

int myPyUnicode_AsWideChar(void *unicode, wchar_t *w, int size) {
    abort();
}

void extabort(void) {
    abort();
}

wchar_t *pythonpath(void) {
    return malloc(48);
}

#ifdef CUSTOM_LIBC
void count_chunks(void* start, void* end, size_t used, void* arg) {
    fprintf(stderr, "    start: %p, end: %p, used: %lu, arg: %p\n", start, end, used, arg);
}

extern struct malloc_state _gm_;
#endif

void inspect_it(void) {
#ifdef CUSTOM_LIBC
    void dlmalloc_inspect_all(void(*handler)(void*, void *, size_t, void*), void* arg);
    fprintf(stderr, "inspecting (dlmalloc is at %p):\n", &_gm_);
    dlmalloc_inspect_all(count_chunks, NULL);
#endif
}
