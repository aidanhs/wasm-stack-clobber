#include <stdint.h>
#include <string.h>
#include "Python.h"

#define DELIM L':'
#define MAXPATHLEN 4096
#define SEP L'/'
static wchar_t exec_prefix[MAXPATHLEN+1];

static int
_Py_wstat(const wchar_t* path, struct stat *buf)
{
    int err;
    char *fname;
    fname = Py_EncodeLocale(path, NULL);
    if (fname == NULL) {
        errno = EINVAL;
        return -1;
    }
    err = stat(fname, buf);
    PyMem_Free(fname);
    return err;
}

static void
reduce(wchar_t *dir)
{
    size_t i = wcslen(dir);
    while (i > 0 && dir[i] != SEP)
        --i;
    dir[i] = '\0';
}

static int
isfile(wchar_t *filename)          /* Is file, not directory */
{
    struct stat buf;
    if (_Py_wstat(filename, &buf) != 0)
        return 0;
    if (!S_ISREG(buf.st_mode))
        return 0;
    return 1;
}


static int
isdir(wchar_t *filename)                   /* Is directory */
{
    struct stat buf;
    if (_Py_wstat(filename, &buf) != 0)
        return 0;
    if (!S_ISDIR(buf.st_mode))
        return 0;
    return 1;
}


/* Add a path component, by appending stuff to buffer.
   buffer must have at least MAXPATHLEN + 1 bytes allocated, and contain a
   NUL-terminated string with no more than MAXPATHLEN characters (not counting
   the trailing NUL).  It's a fatal error if it contains a string longer than
   that (callers must be careful!).  If these requirements are met, it's
   guaranteed that buffer will still be a NUL-terminated string with no more
   than MAXPATHLEN characters at exit.  If stuff is too long, only as much of
   stuff as fits will be appended.
*/
static void
joinpath(wchar_t *buffer, wchar_t *stuff)
{
    size_t n, k;
    if (stuff[0] == SEP)
        n = 0;
    else {
        n = wcslen(buffer);
        if (n > 0 && buffer[n-1] != SEP && n < MAXPATHLEN)
            buffer[n++] = SEP;
    }
    if (n > MAXPATHLEN)
        abort();
    k = wcslen(stuff);
    if (n + k > MAXPATHLEN)
        k = MAXPATHLEN - n;
    wcsncpy(buffer+n, stuff, k);
    buffer[n+k] = '\0';
}

/* copy_absolute requires that path be allocated at least
   MAXPATHLEN + 1 bytes and that p be no more than MAXPATHLEN bytes. */
static void
copy_absolute(wchar_t *path, wchar_t *p, size_t pathlen)
{
    if (p[0] == SEP)
        wcscpy(path, p);
    else {
        if (!_Py_wgetcwd(path, pathlen)) {
            /* unable to get the current directory */
            wcscpy(path, p);
            return;
        }
        if (p[0] == '.' && p[1] == SEP)
            p += 2;
        joinpath(path, p);
    }
}

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
static int
search_for_exec_prefix(wchar_t *argv0_path,
                       wchar_t *_exec_prefix, wchar_t *lib_python)
{
    size_t n;

    /* Check to see if argv[0] is in the build directory. "pybuilddir.txt"
       is written by setup.py and contains the relative path to the location
       of shared library modules. */
    wcsncpy(exec_prefix, argv0_path, MAXPATHLEN);
    exec_prefix[MAXPATHLEN] = L'\0';
    joinpath(exec_prefix, L"pybuilddir.txt");
    if (isfile(exec_prefix)) {
        FILE *f = _Py_wfopen(exec_prefix, L"rb");
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
                    rel_builddir_path[k] = L'\0';
                    wcsncpy(exec_prefix, argv0_path, MAXPATHLEN);
                    exec_prefix[MAXPATHLEN] = L'\0';
                    joinpath(exec_prefix, rel_builddir_path);
                    return -1;
                }
            }
        }
    }

    /* Search from argv0_path, until root is found */
    copy_absolute(exec_prefix, argv0_path, MAXPATHLEN+1);
    do {
        n = wcslen(exec_prefix);
        joinpath(exec_prefix, lib_python);
        joinpath(exec_prefix, L"lib-dynload");
        if (isdir(exec_prefix))
            return 1;
        exec_prefix[n] = L'\0';
        reduce(exec_prefix);
    } while (exec_prefix[0]);

    /* Look at configure's EXEC_PREFIX */
    wcsncpy(exec_prefix, _exec_prefix, MAXPATHLEN);
    exec_prefix[MAXPATHLEN] = L'\0';
    joinpath(exec_prefix, lib_python);
    joinpath(exec_prefix, L"lib-dynload");
    if (isdir(exec_prefix))
        return 1;

    /* Fail */
    return 0;
}

static void
_calculate_path(void)
{
    wchar_t argv0_path[MAXPATHLEN+1];
    argv0_path[0] = '\0';
    wchar_t zip_path[MAXPATHLEN+1];
    int efound; /* 1 if found; -1 if found build directory */
    wchar_t *buf;
    size_t bufsz;
    wchar_t *_pythonpath, *_prefix, *_exec_prefix;
    wchar_t *lib_python;
    fprintf(stderr, "calc 1\n");

    _pythonpath = Py_DecodeLocale(":plat-linux", NULL);
    _prefix = L"/home/aidanhs/Desktop/per/bsaber/bsmeta/plugins/cpython/dist";
    _exec_prefix = L"/home/aidanhs/Desktop/per/bsaber/bsmeta/plugins/cpython/dist";
    lib_python = L"lib/python3.5";
    fprintf(stderr, "calc 2\n");

    if (!_pythonpath || !_prefix || !_exec_prefix || !lib_python) {
        abort();
    }

    fprintf(stderr, "calc 3 =%ls= %p %lu\n", _pythonpath, _pythonpath, wcslen(_pythonpath));

    /* If there is no slash in the argv0 path, then we have to
     * assume python is on the user's $PATH, since there's no
     * other way to find a directory to start the search from.  If
     * $PATH isn't exported, you lose.
     */
    argv0_path[MAXPATHLEN] = '\0';
    fprintf(stderr, "calc 4 =%ls= %p %lu\n", _pythonpath, _pythonpath, wcslen(_pythonpath));

    /* Search for an environment configuration file, first in the
       executable's directory and then in the parent directory.
       If found, open it for use when searching for prefixes.
    */

    fprintf(stderr, "calc 5 =%ls= %p %lu\n", _pythonpath, _pythonpath, wcslen(_pythonpath));
    {
        FILE * env_file = NULL;

        env_file = _Py_wfopen(L"/dev/null", L"r");
        if (env_file == NULL) {
            errno = 0;
            env_file = NULL;
            if (env_file == NULL) {
                errno = 0;
            }
        }
        if (env_file != NULL) {
            /* Look for a 'home' variable and set argv0_path to it, if found */
            if (find_env_config_value(env_file, L"home")) {
                abort();
            }
            fclose(env_file);
            env_file = NULL;
        }
    }

    wcsncpy(zip_path, _prefix, MAXPATHLEN);
    joinpath(zip_path, L"lib/python00.zip");
    fprintf(stderr, "zip_path %lu\n", wcslen(zip_path));

    fprintf(stderr, "calc 6 =%ls= %p %lu\n", _pythonpath, _pythonpath, wcslen(_pythonpath));
    efound = search_for_exec_prefix(argv0_path, _exec_prefix, lib_python);
    if (!efound) {
        fprintf(stderr, "!efound\n");
    }

    bufsz = 200;
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
