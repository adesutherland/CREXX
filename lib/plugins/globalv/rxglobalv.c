/*
 * rxglobalv -- cREXX RXPA adapter over the upstream GLOBALV C API.
 *
 * The direct procedures use globalv.h.  The legacy command procedure is
 * retained for the Level-G ADDRESS facade and uses gv_cmdio so its output is
 * returned in the cREXX array expected by that facade.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crexxpa.h"
#include "globalv.h"
#include "gvint.h"

#ifdef _WIN32
#include <windows.h>
static CRITICAL_SECTION globalv_lock;
static INIT_ONCE globalv_lock_once = INIT_ONCE_STATIC_INIT;
static BOOL CALLBACK globalv_init_lock(PINIT_ONCE once, PVOID parameter,
                                       PVOID *context)
{
    (void)once;
    (void)parameter;
    (void)context;
    InitializeCriticalSection(&globalv_lock);
    return TRUE;
}
static void globalv_enter(void)
{
    InitOnceExecuteOnce(&globalv_lock_once, globalv_init_lock, NULL, NULL);
    EnterCriticalSection(&globalv_lock);
}
static void globalv_leave(void)
{
    LeaveCriticalSection(&globalv_lock);
}
#else
#include <pthread.h>
static pthread_mutex_t globalv_lock = PTHREAD_MUTEX_INITIALIZER;
static void globalv_enter(void)
{
    (void)pthread_mutex_lock(&globalv_lock);
}
static void globalv_leave(void)
{
    (void)pthread_mutex_unlock(&globalv_lock);
}
#endif

typedef struct {
    char **lines;
    int count;
    int capacity;
} globalv_capture;

static int capture_line(globalv_capture *capture, const char *text, int length)
{
    char **grown;
    char *copy;
    int capacity;
    if (length < 0)
        length = (int)strlen(text);
    if (capture->count == capture->capacity) {
        capacity = capture->capacity ? capture->capacity * 2 : 16;
        grown = (char **)realloc(capture->lines,
                                 (size_t)capacity * sizeof(char *));
        if (grown == NULL)
            return 1;
        capture->lines = grown;
        capture->capacity = capacity;
    }
    copy = (char *)malloc((size_t)length + 1);
    if (copy == NULL)
        return 1;
    memcpy(copy, text, (size_t)length);
    copy[length] = '\0';
    capture->lines[capture->count++] = copy;
    return 0;
}

static globalv_capture *active_capture;

static void capture_type(const char *line)
{
    (void)capture_line(active_capture, line, -1);
}

static void capture_error(const char *line)
{
    (void)capture_line(active_capture, line, -1);
}

static int capture_stack(const char *value, int length)
{
    return capture_line(active_capture, value, length);
}

static int capture_fetch(const char *name, char *value, int size, int *length)
{
    (void)name;
    (void)value;
    (void)size;
    (void)length;
    return -1;
}

static int capture_store(const char *name, const char *value, int length)
{
    (void)name;
    return capture_line(active_capture, value, length);
}

static const GV_IO capture_io = {
    capture_type, capture_error, capture_stack, capture_fetch, capture_store
};

static void free_capture(globalv_capture *capture)
{
    int i;
    for (i = 0; i < capture->count; i++)
        free(capture->lines[i]);
    free(capture->lines);
    capture->lines = NULL;
    capture->count = 0;
    capture->capacity = 0;
}

static const char *str(rxpa_attribute_value attribute)
{
    const char *value = GETSTRING(attribute);
    return value ? value : "";
}

static int parse_life(const char *value)
{
    static const char *words[] = { "CALL", "SESSION", "LASTING", "BOTH" };
    char word[16];
    size_t i, length;
    while (*value == ' ')
        value++;
    length = strlen(value);
    while (length > 0 && value[length - 1] == ' ')
        length--;
    if (length == 0)
        return GV_LIFE_CALL;
    if (length >= sizeof word)
        return -1;
    for (i = 0; i < length; i++)
        word[i] = (char)toupper((unsigned char)value[i]);
    word[length] = '\0';
    if (length == 1 && word[0] >= '0' && word[0] <= '3')
        return word[0] - '0';
    for (i = 0; i < 4; i++)
        if (strcmp(word, words[i]) == 0)
            return (int)i;
    return -1;
}

#define GVCHECK(rc) do { \
    int rc_ = (rc); \
    if (rc_ != GV_OK && rc_ != GV_NOTFOUND) \
        RETURNSIGNAL(SIGNAL_FAILURE, (char *)gv_strerror(rc_)); \
} while (0)

PROCEDURE(gvset)
{
    int life = parse_life(str(ARG3));
    int rc;
    if (life < 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "GLOBALV: invalid lifetime (24)");
    globalv_enter();
    rc = gv_setn(str(ARG0), str(ARG1), str(ARG2),
                 (int)strlen(str(ARG2)), life);
    globalv_leave();
    GVCHECK(rc);
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(gvget)
{
    char value[GV_VALLEN + 1];
    int length = 0;
    int rc;
    globalv_enter();
    rc = gv_get(str(ARG0), str(ARG1), value, (int)sizeof value, &length);
    globalv_leave();
    GVCHECK(rc);
    if (rc == GV_NOTFOUND)
        RETURNSTRX((char *)str(ARG2));
    RETURNSTRX(value);
    ENDPROC
}

PROCEDURE(gvexists)
{
    int rc;
    globalv_enter();
    rc = gv_exists(str(ARG0), str(ARG1));
    globalv_leave();
    GVCHECK(rc);
    RETURNINTX(rc);
    ENDPROC
}

PROCEDURE(gvdrop)
{
    int rc;
    globalv_enter();
    rc = gv_drop(str(ARG0), str(ARG1));
    globalv_leave();
    GVCHECK(rc);
    RETURNINTX(rc == GV_OK);
    ENDPROC
}

PROCEDURE(gvpurge)
{
    int rc;
    globalv_enter();
    rc = gv_purge(str(ARG0));
    globalv_leave();
    GVCHECK(rc);
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(gvinit)
{
    int rc;
    globalv_enter();
    rc = gv_init();
    globalv_leave();
    GVCHECK(rc);
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(gvnames)
{
    GV_HANDLE *handle = NULL;
    GV_VAR variable;
    int position = 0;
    int count = 0;
    int rc;
    SETARRAYHI(RETURN, 0);
    globalv_enter();
    rc = gv_open(&handle);
    if (rc == GV_OK) {
        while ((rc = gv_hnext(handle, str(ARG0), &position, &variable)) == GV_OK) {
            count++;
            SETARRAYHI(RETURN, count);
            SETSARRAY(RETURN, count - 1, variable.name);
        }
        gv_abandon(handle);
    }
    globalv_leave();
    GVCHECK(rc);
    PROCRETURN
    ENDPROC
}

PROCEDURE(gvgroups)
{
    GV_HANDLE *handle = NULL;
    char group[GV_GRPLEN + 1];
    int position = 0;
    int count = 0;
    int rc;
    SETARRAYHI(RETURN, 0);
    globalv_enter();
    rc = gv_open(&handle);
    if (rc == GV_OK) {
        while ((rc = gv_hgroups(handle, &position, group)) == GV_OK) {
            count++;
            SETARRAYHI(RETURN, count);
            SETSARRAY(RETURN, count - 1, group);
        }
        gv_abandon(handle);
    }
    globalv_leave();
    GVCHECK(rc);
    PROCRETURN
    ENDPROC
}

PROCEDURE(gvcmd)
{
    int rc;
    fflush(stdout);
    globalv_enter();
    rc = gv_command(str(ARG0));
    globalv_leave();
    fflush(stdout);
    RETURNINTX(rc);
    ENDPROC
}

/* Compatibility surface used by the Level-G GLOBALV ADDRESS facade. */
PROCEDURE(rxglobalv_command)
{
    globalv_capture capture;
    const char *args = str(ARG0);
    int rc;
    int i;
    memset(&capture, 0, sizeof capture);
    globalv_enter();
    active_capture = &capture;
    rc = gv_cmdio(args, (int)strlen(args), &capture_io);
    active_capture = NULL;
    globalv_leave();

    SETARRAYHI(ARG1, 0);
    SETARRAYHI(ARG1, capture.count);
    for (i = 0; i < capture.count; i++)
        SETSTRING(GETATTR(ARG1, i), capture.lines[i]);
    free_capture(&capture);
    RETURNINT(rc);
    RESETSIGNAL
    ENDPROC
}

RXPA_PLUGIN_PROCESS_REENTRANT

LOADFUNCS
    ADDPROC(gvset, "rxglobalv.gvset", "b", ".int",
            "group=.string,name=.string,value=.string,life=.string");
    ADDPROC(gvget, "rxglobalv.gvget", "b", ".string",
            "group=.string,name=.string,default=.string");
    ADDPROC(gvexists, "rxglobalv.gvexists", "b", ".int",
            "group=.string,name=.string");
    ADDPROC(gvdrop, "rxglobalv.gvdrop", "b", ".int",
            "group=.string,name=.string");
    ADDPROC(gvpurge, "rxglobalv.gvpurge", "b", ".int",
            "group=.string");
    ADDPROC(gvnames, "rxglobalv.gvnames", "b", ".string[]",
            "group=.string");
    ADDPROC(gvgroups, "rxglobalv.gvgroups", "b", ".string[]", "");
    ADDPROC(gvinit, "rxglobalv.gvinit", "b", ".int", "");
    ADDPROC(gvcmd, "rxglobalv.gvcmd", "b", ".int",
            "operands=.string");
    ADDPROC(rxglobalv_command, "rxglobalv.command", "b", ".int",
            "args=.string,expose output=.string[]");
ENDLOADFUNCS
