#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXPATHLEN 4096

int isfile(wchar_t *filename);

FILE *myopen(void);

void inspect_it(void);

// all of these abort, but llvm doesn't know that!
void *myPyUnicode_DecodeUTF8(const char *string, int length, const char *errors);
int myPyUnicode_AsWideChar(void *unicode, wchar_t *w, int size);
void extabort(void);

static int
find_env_config_value(FILE * env_file, const wchar_t * key)
{
    extabort(); // ABORTS
    int result = 0; /* meaning not found */
    char buffer[MAXPATHLEN*2+1];  /* allow extra for key, '=', etc. */

    fseek(env_file, 0, SEEK_SET);
    while (!feof(env_file)) {
        char * p = fgets(buffer, MAXPATHLEN*2, env_file);
        wchar_t tmpbuffer[MAXPATHLEN*2+1];
        void * decoded;
        int n = 0;

        if (p == NULL)
            break;
        decoded = myPyUnicode_DecodeUTF8(buffer, n, "surrogateescape");
        if (decoded != NULL) {
            int k;
            k = myPyUnicode_AsWideChar(decoded,
                                     tmpbuffer, MAXPATHLEN * 2);
            abort();
        }
    }
    return result;
}

/* search_for_exec_prefix requires that argv0_path be no more than
   MAXPATHLEN bytes long.
*/
static wchar_t exec_prefix[MAXPATHLEN+1];

static void
search_for_exec_prefix(wchar_t *argv0_path)
{
    if (isfile(exec_prefix)) {
        extabort(); // ABORTS
        size_t n;
        FILE *f = myopen();
        if (f != NULL) {
            char buf[MAXPATHLEN+1];
            void *decoded;
            wchar_t rel_builddir_path[MAXPATHLEN+1];
            n = fread(buf, 1, MAXPATHLEN, f);
            buf[n] = '\0';
            fclose(f);
            decoded = myPyUnicode_DecodeUTF8(buf, n, "surrogateescape");
            if (decoded != NULL) {
                int k;
                k = myPyUnicode_AsWideChar(decoded,
                                         rel_builddir_path, MAXPATHLEN);
                abort();
            }
        }
    }
    wcscpy(exec_prefix, argv0_path);
}

static void
_calculate_path(void)
{
    wchar_t argv0_path[MAXPATHLEN+1];
    argv0_path[0] = L'/';
    argv0_path[1] = L'x';
    argv0_path[2] = L'\0';
    wchar_t zip_path[MAXPATHLEN+1];
    wchar_t *buf;
    size_t bufsz;

    wchar_t *pythonpath(void);
    wchar_t *_pythonpath = pythonpath();

    fprintf(stderr, "calc 3 %p\n", _pythonpath);

    inspect_it();
    {
        FILE * env_file = myopen();
        if (env_file != NULL) {
            extabort(); // ABORTS
            if (find_env_config_value(env_file, L"home")) {
                abort();
            }
        }
    }

    inspect_it();

    wcsncpy(zip_path, L"/x.zip", MAXPATHLEN);
    //wcscpy(zip_path, L"/x.zip");
    fprintf(stderr, "zip_path %p-%p %ls %lu\n", zip_path, zip_path + MAXPATHLEN+1, zip_path, wcslen(zip_path));

    inspect_it();

    search_for_exec_prefix(argv0_path);

    bufsz = sizeof(wchar_t)*8000;
    buf = malloc(bufsz);
    if (buf == NULL) abort();
    fprintf(stderr, "calc 7 %p to %p %lu\n", buf, buf+bufsz, bufsz);
    if (buf < _pythonpath && _pythonpath < buf+bufsz) {
        fprintf(stderr, "we have an overlap!\n");
        inspect_it();
    }
}

int main(int argc, char **argv) {
    _calculate_path();
    return 0;
}
