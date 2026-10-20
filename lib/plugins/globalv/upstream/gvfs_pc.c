/*
 * GVFS_PC C -- GLOBALV (VM/370plus): the PC platform (Linux, macOS,
 * Windows) of the library: the stores and the default GV_IO.  C89 plus
 * a few POSIX / Win32 calls.
 *
 * Where the stores are (the record format is that of CMS, in the PC's
 * character set):
 *   LASTING  $GLOBALV_HOME/lasting.globalv
 *            default ~/.globalv            (Windows: %APPDATA%\globalv)
 *   SESSION  $GLOBALV_SESSION/session.globalv, the table there too
 *            default $XDG_RUNTIME_DIR/globalv (gone at logout), else
 *            $TMPDIR or /tmp /globalv-<uid>  (Windows: %TEMP%\globalv)
 * The default GV_IO (the globalv command, gv_command): LIST and GRPLIST
 * to stdout, messages to stderr, STACK values to stdout, PUT from the
 * environment, GET as assignments for the shell to evaluate
 * (GLOBALV_SHELL = sh, cmd or ps).
 *
 * Build 2, 19 October 2026.  (Build 1: in GVPC C.)
 */
#include <stddef.h>          /* first: gcclib31 string.h needs size_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gvint.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define GV_SEP      '\\'
#define gv_mkdir(p) _mkdir(p)
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define GV_SEP      '/'
#define gv_mkdir(p) mkdir((p), 0700)
#endif

#define GV_PATHLEN  1024

static char  lastdir[GV_PATHLEN], sessdir[GV_PATHLEN];
static FILE *rf, *wf;
static char  wpath[GV_PATHLEN], wtmp[GV_PATHLEN + 4];

/* ---------------------------------------------------------------- */
/* the directories and files                                        */
/* ---------------------------------------------------------------- */

static void join(char *out, const char *a, const char *b)
{
    size_t n = strlen(a);
    if (n + 1 + strlen(b) + 1 > GV_PATHLEN) {
        out[0] = '\0';
        return;
    }
    strcpy(out, a);
    if (n > 0 && out[n - 1] != GV_SEP && out[n - 1] != '/')
        out[n++] = GV_SEP;
    strcpy(out + n, b);
}

static int dirsdone;

static void dirs(void)
{
    const char *e;
    if ((e = getenv("GLOBALV_HOME")) != NULL && *e)
        join(lastdir, e, "");
#ifdef _WIN32
    else if ((e = getenv("APPDATA")) != NULL && *e)
        join(lastdir, e, "globalv");
    else if ((e = getenv("USERPROFILE")) != NULL && *e)
        join(lastdir, e, ".globalv");
#else
    else if ((e = getenv("HOME")) != NULL && *e)
        join(lastdir, e, ".globalv");
#endif
    else
        join(lastdir, ".", ".globalv");

    if ((e = getenv("GLOBALV_SESSION")) != NULL && *e)
        join(sessdir, e, "");
#ifdef _WIN32
    else if ((e = getenv("TEMP")) != NULL && *e)
        join(sessdir, e, "globalv");
    else
        join(sessdir, lastdir, "session");
#else
    else if ((e = getenv("XDG_RUNTIME_DIR")) != NULL && *e)
        join(sessdir, e, "globalv");
    else {
        char d[64];
#ifdef __wasi__
        strcpy(d, "globalv");       /* WASI: no users, no getuid */
#else
        sprintf(d, "globalv-%lu", (unsigned long)getuid());
#endif
        e = getenv("TMPDIR");
        join(sessdir, (e != NULL && *e) ? e : "/tmp", d);
    }
#endif
}

static void path(int store, char *out)
{
    if (!dirsdone) {
        dirs();
        dirsdone = 1;
    }
    switch (store) {
    case GVS_LASTING: join(out, lastdir, "lasting.globalv"); break;
    case GVS_SESSION: join(out, sessdir, "session.globalv"); break;
    default:         join(out, sessdir, "table.globalv");   break;
    }
}

/* ---------------------------------------------------------------- */
/* the stores                                                       */
/* ---------------------------------------------------------------- */

int gvs_ropen(int store)
{
    char p[GV_PATHLEN];
    path(store, p);
    rf = fopen(p, "rb");
    return rf == NULL ? 1 : 0;
}

int gvs_read(char *rec)
{
    size_t n = fread(rec, 1, GV_RECLEN, rf);
    if (n == GV_RECLEN)
        return 1;
    return n == 0 ? 0 : -1;
}

void gvs_rclose(void)
{
    if (rf != NULL)
        fclose(rf);
    rf = NULL;
}

int gvs_wopen(int store)
{
    path(store, wpath);
    gv_mkdir(store == GVS_LASTING ? lastdir : sessdir);  /* may exist */
    strcpy(wtmp, wpath);
    strcat(wtmp, ".new");
    wf = fopen(wtmp, "wb");
    return wf == NULL ? 1 : 0;
}

int gvs_write(const char *rec)
{
    return fwrite(rec, 1, GV_RECLEN, wf) == GV_RECLEN ? 0 : 1;
}

int gvs_wclose(void)
{
    int bad = ferror(wf);
    if (fclose(wf) != 0)
        bad = 1;
    wf = NULL;
    if (bad) {
        remove(wtmp);
        return 1;
    }
#ifdef _WIN32
    remove(wpath);              /* Windows rename does not replace */
#endif
    return rename(wtmp, wpath) == 0 ? 0 : 1;
}

int gvs_erase(int store)
{
    char p[GV_PATHLEN];
    path(store, p);
    remove(p);
    return 0;
}

/* ---------------------------------------------------------------- */
/* the terminal, the stack, the variables                           */
/* ---------------------------------------------------------------- */

static void io_type(const char *line)
{
    puts(line);
}

static void io_error(const char *line)
{
    fflush(stdout);
    fprintf(stderr, "%s\n", line);
}

static int io_stack(const char *s, int len)
{
    fwrite(s, 1, (size_t)len, stdout);
    putchar('\n');
    return 0;
}

static int io_fetch(const char *name, char *buf, int size, int *len)
{
    const char *v = getenv(name);
    int         n;
    if (v == NULL)
        return 1;
    n = (int)strlen(v);
    if (n > size - 1)
        n = size - 1;
    memcpy(buf, v, (size_t)n);
    buf[n] = '\0';
    *len = n;
    return 0;
}

static int io_store(const char *name, const char *val, int len)
{
    const char *sh = getenv("GLOBALV_SHELL");
    int         i;
#ifdef _WIN32
    if (sh == NULL || *sh == '\0')
        sh = "cmd";
#else
    if (sh == NULL || *sh == '\0')
        sh = "sh";
#endif
    if (strcmp(sh, "cmd") == 0) {
        printf("set \"%s=", name);
        fwrite(val, 1, (size_t)len, stdout);
        printf("\"\n");
    } else if (strcmp(sh, "ps") == 0) {
        printf("$env:%s = '", name);
        for (i = 0; i < len; i++) {
            if (val[i] == '\'')
                putchar('\'');
            putchar(val[i]);
        }
        printf("'\n");
    } else {
        printf("%s='", name);
        for (i = 0; i < len; i++) {
            if (val[i] == '\'')
                fputs("'\\''", stdout);
            else
                putchar(val[i]);
        }
        printf("'\n");
    }
    return 0;
}

/* ---------------------------------------------------------------- */
/* the lock: lock.globalv in the session directory, held from        */
/* gv_open to gv_close, so a read-change-write is not lost            */
/* ---------------------------------------------------------------- */

static int lockdepth;
#ifdef _WIN32
static HANDLE lockh = INVALID_HANDLE_VALUE;
#else
static int lockfd = -1;
#endif

int gvs_lock(void)
{
    char p[GV_PATHLEN];
    if (lockdepth++ > 0)
        return 0;                   /* nested handles in one process */
    path(GVS_TABLE, p);             /* sets the directories          */
    join(p, sessdir, "lock.globalv");
    gv_mkdir(sessdir);
#ifdef _WIN32
    {
        OVERLAPPED ov;
        lockh = CreateFileA(p, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (lockh == INVALID_HANDLE_VALUE)
            return 0;               /* no lock rather than no GLOBALV */
        memset(&ov, 0, sizeof ov);
        LockFileEx(lockh, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &ov);
    }
#elif defined(__wasi__)
    /* WASI: one process per file system, and no fcntl locks */
#else
    {
        struct flock fl;
        lockfd = open(p, O_RDWR | O_CREAT, 0600);
        if (lockfd < 0)
            return 0;
        memset(&fl, 0, sizeof fl);
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        while (fcntl(lockfd, F_SETLKW, &fl) != 0)
            if (errno != EINTR)
                break;              /* e.g. no locks on NFS: go on */
    }
#endif
    return 0;
}

void gvs_unlock(void)
{
    if (lockdepth == 0 || --lockdepth > 0)
        return;
#ifdef _WIN32
    if (lockh != INVALID_HANDLE_VALUE) {
        CloseHandle(lockh);         /* releases the lock */
        lockh = INVALID_HANDLE_VALUE;
    }
#else
    if (lockfd >= 0) {
        close(lockfd);              /* releases the lock */
        lockfd = -1;
    }
#endif
}

const GV_IO gv_io_default = { io_type, io_error, io_stack, io_fetch, io_store };
