/*
 * GLOBALV H -- GLOBALV (VM/370plus): the C API.  C89, C++ safe.
 *
 * Global variables shared between programs and languages: the same
 * table, files and lifetimes as the GLOBALV command, so a value set by
 * an EXEC is seen by a C, COBOL, Python ... program and back.
 *
 *   group  up to 8 characters,  NULL or "" = "UNNAMED"   (upper-cased)
 *   name   1 to 32 characters                            (upper-cased)
 *   value  0 to 255 bytes; a null (empty) value drops the variable
 *   life   GV_LIFE_CALL     until GLOBALV INIT (the next CMS IPL)  SET
 *          GV_LIFE_SESSION  ... and in SESSION GLOBALV             SETS
 *          GV_LIFE_LASTING  ... and in LASTING GLOBALV (kept)      SETL
 *          GV_LIFE_BOTH     session and lasting                    SETP
 *
 * Two ways to call it:
 *   one-shot  gv_set, gv_get, ...  each reads the table and writes back
 *             what changed: simple, for a few calls
 *   handle    gv_open, gv_h..., gv_close: one read, one write, for many
 *             calls (a LIST, a loop).  Nothing is written before close.
 *
 * Return codes (also the GLOBALV command's):
 *   GV_OK 0, GV_NOTFOUND 1 (get: no such variable; next: the end),
 *   GV_EBADARG 24 (name, group or value invalid or too long),
 *   GV_EFULL 28 (table full), GV_ESTORE 32 (a store file failed),
 *   GV_ENOMEM 104, GV_ENOTAVAIL -1 (a binding built without GLOBALV).
 *
 * Build 2, 19 October 2026.
 */
#ifndef GLOBALV_H
#define GLOBALV_H

#ifdef __cplusplus
extern "C" {
#endif

#define GV_API_VERSION  1               /* bumped on incompatible change */

#define GV_GRPLEN       8
#define GV_NAMLEN       32
#define GV_VALLEN       255

#define GV_LIFE_CALL    0
#define GV_LIFE_SESSION 1
#define GV_LIFE_LASTING 2
#define GV_LIFE_BOTH    3

#define GV_OK           0
#define GV_NOTFOUND     1
#define GV_EBADARG      24
#define GV_EFULL        28
#define GV_ESTORE       32
#define GV_ENOMEM       104
#define GV_ENOTAVAIL    (-1)            /* for bindings built without GLOBALV */

typedef struct gv_handle GV_HANDLE;

/* One variable, as gv_hnext returns it.                               */
typedef struct {
    char group[GV_GRPLEN + 1];
    char name[GV_NAMLEN + 1];
    char value[GV_VALLEN + 1];          /* NUL ended (may hold NULs)    */
    int  vlen;
    int  life;
} GV_VAR;

/* ---- one-shot calls -------------------------------------------------- */

/* value NUL ended; gv_setn takes a length (binary values).            */
int gv_set (const char *group, const char *name, const char *value, int life);
int gv_setn(const char *group, const char *name, const char *value, int len,
            int life);

/* The value into buf (size bytes, NUL ended, truncated to fit); *len
 * (may be NULL) gets its full length.  GV_NOTFOUND: buf = "".         */
int gv_get (const char *group, const char *name, char *buf, int size, int *len);

/* 1 set, 0 not set, else the error rc (GV_EBADARG ...)               */
int gv_exists(const char *group, const char *name);
int gv_drop (const char *group, const char *name);
int gv_purge(const char *group);        /* all variables of the group  */
int gv_init (void);                     /* a new session: SESSION GLOBALV
                                           erased, the table from LASTING */

/* Run a GLOBALV command ("SELECT FULIST LIST").  Output goes to the
 * platform's terminal; GET/PUT/STACK use the platform's variables and
 * stack (on the PC: stdout and the environment).                      */
int gv_command(const char *operands);

const char *gv_version(void);           /* "GLOBALV 3 (VM/370plus)"     */

/* "GLOBALV: invalid name, group or value (24)" ... for any rc; the text
 * every binding uses for its errors.                                   */
const char *gv_strerror(int rc);

/* ---- handle calls ---------------------------------------------------- */

int gv_open (GV_HANDLE **h);
int gv_close(GV_HANDLE *h);             /* writes, frees; rc of the write */
void gv_abandon(GV_HANDLE *h);          /* frees, writes nothing        */

int gv_hset (GV_HANDLE *h, const char *group, const char *name,
             const char *value, int len, int life);
int gv_hget (GV_HANDLE *h, const char *group, const char *name,
             char *buf, int size, int *len);
int gv_hdrop (GV_HANDLE *h, const char *group, const char *name);
int gv_hpurge(GV_HANDLE *h, const char *group);
int gv_hinit (GV_HANDLE *h);

/* Iterate: *pos = 0 to start.  group NULL: every group, in table order;
 * else that group only.  GV_OK with *v filled, GV_NOTFOUND at the end. */
int gv_hnext(GV_HANDLE *h, const char *group, int *pos, GV_VAR *v);

/* The distinct group names, *pos = 0 to start.  grp: GV_GRPLEN+1.     */
int gv_hgroups(GV_HANDLE *h, int *pos, char *grp);

int gv_hcount(GV_HANDLE *h, const char *group);   /* NULL: all        */

#ifdef __cplusplus
}
#endif
#endif
