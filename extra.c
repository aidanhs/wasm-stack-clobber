#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>

int
isfile(wchar_t *filename)
{
    return 0;
}

FILE *myopen(wchar_t *filename, wchar_t *mode) {
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
