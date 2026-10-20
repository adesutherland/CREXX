/*
 * GVCMD C -- GLOBALV (VM/370plus): the GLOBALV command, on the C API.
 * C89.
 *
 *   GLOBALV [INIT] [SELECT group] verb ...
 *     SET   name value    the value until GLOBALV INIT
 *     SETS  name value    ... and in SESSION GLOBALV
 *     SETL  name value    ... and in LASTING GLOBALV (kept)
 *     SETP  name value    SETS and SETL
 *     PUT   name ...      the caller's variables' values
 *     PUTS, PUTL, PUTP    likewise, with the lifetimes above
 *     GET   name ...      set the caller's variables
 *     STACK name ...      the values to the stack (FIFO)
 *     LIST  [name ...]    type the group's variables
 *     PURGE               drop the group's variables
 *     GRPLIST             type the group names
 *   The group is UNNAMED unless SELECT names one.  A null value drops
 *   the variable.  Names up to 32 characters, values up to 255, groups
 *   up to 8.  Return codes: 0, 24 syntax, 28 table full, 32 store
 *   error, negative: no variable interface (from the platform).
 *
 * Build 2, 19 October 2026.  (Build 1: in GVCORE C.)
 */
#include <stddef.h>          /* first: gcclib31 string.h needs size_t */
#include <string.h>
#include "gvint.h"

static const char MSGSYN[]  = "DMSGLB024E GLOBALV: INVALID OR MISSING OPERAND";
static const char MSGLEN[]  = "DMSGLB024E GLOBALV: NAME OR VALUE TOO LONG";
static const char MSGFULL[] = "DMSGLB028E GLOBALV: NO ROOM FOR MORE VARIABLES";
static const char MSGFILE[] = "DMSGLB032E GLOBALV: ERROR WRITING THE GLOBALV FILES";

typedef struct {
    GV_HANDLE   *h;
    const GV_IO *io;
    const char  *p, *end;
    char         group[GV_GRPLEN + 1];
} CMD;

static int nextword(CMD *c, const char **w)
{
    while (c->p < c->end && *c->p == ' ')
        c->p++;
    *w = c->p;
    while (c->p < c->end && *c->p != ' ')
        c->p++;
    return (int)(c->p - *w);
}

static int rest(CMD *c, const char **w)
{
    const char *e = c->end;
    while (c->p < e && *c->p == ' ')
        c->p++;
    while (e > c->p && e[-1] == ' ')
        e--;
    *w = c->p;
    c->p = c->end;
    return (int)(e - *w);
}

static int keyword(const char *w, int n, const char *k)
{
    int i;
    if ((int)strlen(k) != n)
        return 0;
    for (i = 0; i < n; i++) {
        char ch = w[i];
        if (ch >= 'a' && ch <= 'z')
            ch = (char)(ch - 'a' + 'A');
        if (ch != k[i])
            return 0;
    }
    return 1;
}

/* A word as a NUL-ended name (the API checks and upper-cases it). */
static int word(CMD *c, char *out, int max, int *rc)
{
    const char *w;
    int         n = nextword(c, &w);
    *rc = 0;
    if (n == 0)
        return 0;
    if (n > max) {
        c->io->error(MSGLEN);
        *rc = GV_EBADARG;
        return 0;
    }
    memcpy(out, w, (size_t)n);
    out[n] = '\0';
    return n;
}

static int apirc(CMD *c, int rc)
{
    if (rc == GV_EBADARG)
        c->io->error(MSGLEN);
    else if (rc == GV_EFULL)
        c->io->error(MSGFULL);
    return rc;
}

static int doset(CMD *c, int life)
{
    char        name[GV_NAMLEN + 1];
    const char *v;
    int         n, rc;
    if (!word(c, name, GV_NAMLEN, &rc)) {
        if (rc == 0)
            c->io->error(MSGSYN);
        return GV_EBADARG;
    }
    n = rest(c, &v);
    if (n > GV_VALLEN) {
        c->io->error(MSGLEN);
        return GV_EBADARG;
    }
    return apirc(c, gv_hset(c->h, c->group, name, v, n, life));
}

static int doput(CMD *c, int life)
{
    char name[GV_NAMLEN + 1], val[GV_VALLEN + 1];
    int  rc, n = 0, len;
    while (word(c, name, GV_NAMLEN, &rc)) {
        char up[GV_NAMLEN + 1];
        int  i;
        for (i = 0; name[i]; i++)       /* variables by their upper name */
            up[i] = (char)((name[i] >= 'a' && name[i] <= 'z')
                           ? name[i] - 'a' + 'A' : name[i]);
        up[i] = '\0';
        n++;
        len = 0;
        rc = c->io->fetch(up, val, (int)sizeof val, &len);
        if (rc < 0)
            return rc;
        if (rc == 1 || len < 0)
            len = 0;                    /* not set in the caller: null */
        if (len > GV_VALLEN)
            len = GV_VALLEN;
        rc = apirc(c, gv_hset(c->h, c->group, up, val, len, life));
        if (rc)
            return rc;
    }
    if (rc)
        return rc;
    if (n == 0) {
        c->io->error(MSGSYN);
        return GV_EBADARG;
    }
    return 0;
}

static int doget(CMD *c, int stack)
{
    char name[GV_NAMLEN + 1], val[GV_VALLEN + 1];
    int  rc, n = 0, len;
    while (word(c, name, GV_NAMLEN, &rc)) {
        char up[GV_NAMLEN + 1];
        int  i;
        for (i = 0; name[i]; i++)
            up[i] = (char)((name[i] >= 'a' && name[i] <= 'z')
                           ? name[i] - 'a' + 'A' : name[i]);
        up[i] = '\0';
        n++;
        if (gv_hget(c->h, c->group, up, val, (int)sizeof val, &len) != GV_OK)
            len = 0;
        rc = stack ? c->io->stack(val, len) : c->io->store(up, val, len);
        if (rc)
            return rc;
    }
    if (rc)
        return rc;
    if (n == 0) {
        c->io->error(MSGSYN);
        return GV_EBADARG;
    }
    return 0;
}

static void typevar(CMD *c, const GV_VAR *v)
{
    char line[GV_GRPLEN + 1 + GV_NAMLEN + 3 + GV_VALLEN + 1];
    int  n = (int)strlen(v->group);
    memset(line, ' ', GV_GRPLEN + 1);
    memcpy(line, v->group, (size_t)n);
    strcpy(line + GV_GRPLEN + 1, v->name);
    strcat(line, " = ");
    strcat(line, v->value);
    c->io->type(line);
}

static int dolist(CMD *c)
{
    char   name[GV_NAMLEN + 1];
    GV_VAR v;
    int    rc, named = 0, pos = 0;
    while (word(c, name, GV_NAMLEN, &rc)) {
        named = 1;
        if (gv_hget(c->h, c->group, name, v.value, (int)sizeof v.value,
                    &v.vlen) == GV_OK) {
            int i;
            strcpy(v.group, c->group);
            for (i = 0; name[i]; i++)
                v.name[i] = (char)((name[i] >= 'a' && name[i] <= 'z')
                                   ? name[i] - 'a' + 'A' : name[i]);
            v.name[i] = '\0';
            typevar(c, &v);
        }
    }
    if (rc)
        return rc;
    if (!named)
        while (gv_hnext(c->h, c->group, &pos, &v) == GV_OK)
            typevar(c, &v);
    return 0;
}

static int dogrplist(CMD *c)
{
    char g[GV_GRPLEN + 1];
    int  pos = 0;
    while (gv_hgroups(c->h, &pos, g) == GV_OK)
        c->io->type(g);
    return 0;
}

static int verb(CMD *c)
{
    const char *w;
    int         n = nextword(c, &w);

    if (n > 0 && keyword(w, n, "INIT")) {
        gv_hinit(c->h);
        n = nextword(c, &w);
        if (n == 0)
            return 0;
    }
    if (n > 0 && keyword(w, n, "SELECT")) {
        int i;
        n = nextword(c, &w);
        if (n == 0) {
            c->io->error(MSGSYN);
            return GV_EBADARG;
        }
        if (n > GV_GRPLEN) {
            c->io->error(MSGLEN);
            return GV_EBADARG;
        }
        for (i = 0; i < n; i++)
            c->group[i] = (char)((w[i] >= 'a' && w[i] <= 'z')
                                 ? w[i] - 'a' + 'A' : w[i]);
        c->group[n] = '\0';
        n = nextword(c, &w);
    }
    if (n == 0)
        ;
    else if (keyword(w, n, "SET"))     return doset(c, GV_LIFE_CALL);
    else if (keyword(w, n, "SETS"))    return doset(c, GV_LIFE_SESSION);
    else if (keyword(w, n, "SETL"))    return doset(c, GV_LIFE_LASTING);
    else if (keyword(w, n, "SETP"))    return doset(c, GV_LIFE_BOTH);
    else if (keyword(w, n, "PUT"))     return doput(c, GV_LIFE_CALL);
    else if (keyword(w, n, "PUTS"))    return doput(c, GV_LIFE_SESSION);
    else if (keyword(w, n, "PUTL"))    return doput(c, GV_LIFE_LASTING);
    else if (keyword(w, n, "PUTP"))    return doput(c, GV_LIFE_BOTH);
    else if (keyword(w, n, "GET"))     return doget(c, 0);
    else if (keyword(w, n, "STACK"))   return doget(c, 1);
    else if (keyword(w, n, "LIST"))    return dolist(c);
    else if (keyword(w, n, "PURGE"))   return gv_hpurge(c->h, c->group);
    else if (keyword(w, n, "GRPLIST")) return dogrplist(c);
    c->io->error(MSGSYN);
    return GV_EBADARG;
}

int gv_cmdio(const char *operands, int len, const GV_IO *io)
{
    CMD c;
    int rc, wrc;

    memset(&c, 0, sizeof c);
    c.io = io ? io : &gv_io_default;
    strcpy(c.group, "UNNAMED");
    c.p = operands;
    c.end = operands + (len < 0 ? 0 : len);
    rc = gv_open(&c.h);
    if (rc != GV_OK)
        return rc;
    rc = verb(&c);
    wrc = gv_close(c.h);
    if (wrc != GV_OK) {
        c.io->error(MSGFILE);
        if (rc == 0)
            rc = wrc;
    }
    return rc;
}

int gv_command(const char *operands)
{
    return gv_cmdio(operands, operands ? (int)strlen(operands) : 0, NULL);
}
