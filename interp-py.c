#include <stdint.h>
#include <string.h>
#include "Python.h"

#define DELIM L':'
#define MAXPATHLEN 4096
#define SEP L'/'

int isfile(wchar_t *filename);

int isdir(wchar_t *filename);

FILE *myopen(wchar_t *filename, wchar_t *mode);

/* search for a prefix value in an environment file. If found, copy it
   to the provided buffer, which is expected to be no more than MAXPATHLEN
   bytes long.
*/

static int
find_env_config_value(FILE * env_file, const wchar_t * key)
{
    int result = 0; /* meaning not found */
    char buffer[MAXPATHLEN*2+1];  /* allow extra for key, '=', etc. */

    fseek(env_file, 0, SEEK_SET);
    while (!feof(env_file)) {
        char * p = fgets(buffer, MAXPATHLEN*2, env_file);
        wchar_t tmpbuffer[MAXPATHLEN*2+1];
        PyObject * decoded;
        int n;

        if (p == NULL)
            break;
        n = strlen(p);
        if (p[n - 1] != '\n') {
            /* line has overflowed - bail */
            break;
        }
        if (p[0] == '#')    /* Comment - skip */
            continue;
        decoded = PyUnicode_DecodeUTF8(buffer, n, "surrogateescape");
        if (decoded != NULL) {
            Py_ssize_t k;
            k = PyUnicode_AsWideChar(decoded,
                                     tmpbuffer, MAXPATHLEN * 2);
            Py_DECREF(decoded);
            if (k >= 0) {
                abort();
            }
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
            char buf[MAXPATHLEN+1];
            PyObject *decoded;
            wchar_t rel_builddir_path[MAXPATHLEN+1];
            n = fread(buf, 1, MAXPATHLEN, f);
            buf[n] = '\0';
            fclose(f);
            decoded = PyUnicode_DecodeUTF8(buf, n, "surrogateescape");
            if (decoded != NULL) {
                Py_ssize_t k;
                k = PyUnicode_AsWideChar(decoded,
                                         rel_builddir_path, MAXPATHLEN);
                Py_DECREF(decoded);
                if (k >= 0) {
                    abort();
                }
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
    buf = PyMem_New(wchar_t, bufsz);
    if (buf == NULL) {
        Py_FatalError(
            "Not enough memory for dynamic PYTHONPATH");
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
    int ret;

    //// Because loading encodings doesn't work :/
    //Py_FileSystemDefaultEncoding = "utf-8";

    ret = setenv("PYTHONHASHSEED", "0", 1);
    if (ret != 0) {
        perror("set python hash seed");
        return ret;
    }
    // TODO: don't imply 'import site' on initialization
    ret = setenv("PYTHONNOUSERSITE", "1", 1);
    if (ret != 0) {
        perror("set python user site");
        return ret;
    }
    ret = setenv("PYTHONDONTWRITEBYTECODE", "1", 1);
    if (ret != 0) {
        perror("set python no write bytecode");
        return ret;
    }
    ret = setenv("PYTHONDEBUG", "1", 1);
    if (ret != 0) {
        perror("set python debug");
        return ret;
    }
    ret = setenv("PYTHONVERBOSE", "1", 1);
    if (ret != 0) {
        perror("set python verbose");
        return ret;
    }
    //ret = setenv("PYTHONHOME", "/homeless", 1);
    //if (ret != 0) {
    //    perror("set python home");
    //    return ret;
    //}
    ret = setenv("PYTHONPATH", "/work/lib.zip", 1);
    if (ret != 0) {
        perror("set python path");
        return ret;
    }
    _calculate_path();
    return 0;
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    return run_script();
}
