#include <stdint.h>
#include <string.h>
#include "Python.h"

#define DELIM L':'
#define MAXPATHLEN 4096
#define SEP L'/'

int isfile(wchar_t *filename);

FILE *myopen(wchar_t *filename, wchar_t *mode);

/* search for a prefix value in an environment file. If found, copy it
   to the provided buffer, which is expected to be no more than MAXPATHLEN
   bytes long.
*/

// all of these abort, but llvm doesn't know that!
void *myPyUnicode_DecodeUTF8(const char *string, int length, const char *errors);
int myPyUnicode_AsWideChar(void *unicode, wchar_t *w, int size);
void extabort(void);

static int
find_env_config_value(FILE * env_file, const wchar_t * key)
{
    int result = 0; /* meaning not found */
    char buffer[MAXPATHLEN*2+1];  /* allow extra for key, '=', etc. */

    fseek(env_file, 0, SEEK_SET);
    while (!feof(env_file)) {
        extabort();
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

static int
search_for_exec_prefix(wchar_t *argv0_path)
{
    size_t n;

    /* Check to see if argv[0] is in the build directory. "pybuilddir.txt"
       is written by setup.py and contains the relative path to the location
       of shared library modules. */
    exec_prefix[0] = L'\0';
    if (isfile(exec_prefix)) {
        FILE *f = myopen(exec_prefix, L"rb");
        if (f == NULL)
            errno = 0;
        else {
            extabort();
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

    /* Search from argv0_path, until root is found */
    wcscpy(exec_prefix, argv0_path);

    return 0;
}

static void
_calculate_path(void)
{
    wchar_t argv0_path[MAXPATHLEN+1];
    argv0_path[0] = L'/';
    argv0_path[1] = L'x';
    argv0_path[2] = L'\0';
    wchar_t zip_path[MAXPATHLEN+1];
    int efound; /* 1 if found; -1 if found build directory */
    wchar_t *buf;
    size_t bufsz;
    wchar_t *_pythonpath, *_prefix;
    fprintf(stderr, "calc 1\n");

    _pythonpath = Py_DecodeLocale(":plat-linux", NULL);
    _prefix = L"/home/aidanhs/Desktop/per/bsaber/bsmeta/plugins/cpython/dist";
    fprintf(stderr, "calc 2\n");

    if (!_pythonpath || !_prefix) {
        abort();
    }

    fprintf(stderr, "calc 3 =%ls= %p %lu\n", _pythonpath, _pythonpath, wcslen(_pythonpath));

    {
        FILE * env_file = NULL;

        env_file = myopen(L"/dev/null", L"r");
        if (env_file == NULL) {
            errno = 0;
        } else {
            /* Look for a 'home' variable and set argv0_path to it, if found */
            if (find_env_config_value(env_file, L"home")) {
                abort();
            }
            fclose(env_file);
            env_file = NULL;
        }
    }

    wcsncpy(zip_path, _prefix, MAXPATHLEN);
    fprintf(stderr, "zip_path %lu\n", wcslen(zip_path));

    fprintf(stderr, "calc 6 =%ls= %p %lu\n", _pythonpath, _pythonpath, wcslen(_pythonpath));
    efound = search_for_exec_prefix(argv0_path);
    if (!efound) {
        fprintf(stderr, "!efound\n");
    }

    bufsz = 8000;
    buf = malloc(sizeof(wchar_t)*bufsz);
    if (buf == NULL) {
        abort();
    }
    buf[0] = '\0';
    fprintf(stderr, "calc 7 =%ls= %p %lu\n", _pythonpath, _pythonpath, wcslen(_pythonpath));
    fprintf(stderr, "calc 7 %p to %p %lu\n", buf, buf+bufsz, bufsz);
    if (buf < _pythonpath && _pythonpath < buf+bufsz) {
        fprintf(stderr, "we have an overlap!\n");
    }
}

void calculate_path();

int run_script() {
    _calculate_path();
    return 0;
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    return run_script();
}
