/*
 * GVCORE C -- GLOBALV (VM/370plus): the table and the C API (GLOBALV H).
 * Portable C89; no static data that changes.  The stores are behind
 * GVINT H (gvfs_pc.c, gvfs_cms.c); the command is in gvcmd.c.
 *
 * Build 2, 19 October 2026.  (Build 1 had the command in this file.)
 */
#include <stddef.h>          /* first: gcclib31 string.h needs size_t */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gvint.h"

#ifndef GV_MAXENT
#define GV_MAXENT   4096
#endif

#define F_USED      0x80

typedef struct {
    char          grp[GV_GRPLEN + 1];
    unsigned char life;
    char          name[GV_NAMLEN + 1];
    int           vlen;
    char          val[GV_VALLEN + 1];
} GVENT;

struct gv_handle {
    GVENT        *ent;
    int           n, cap;
    unsigned char dirty;            /* stores to rewrite: lifetime bits */
    int           changed;          /* the table to rewrite             */
};

/* ---------------------------------------------------------------- */
/* names                                                            */
/* ---------------------------------------------------------------- */

/* s upper case into out (max characters); NULL/"" -> dflt.  0 or 24. */
static int norm(const char *s, char *out, int max, const char *dflt)
{
    int i, n;
    if (s != NULL)
        while (*s == ' ')
            s++;
    if (s == NULL || *s == '\0')
        s = dflt;                       /* NULL, "" or blanks: the default */
    if (s == NULL)
        return GV_EBADARG;
    n = (int)strlen(s);
    while (n > 0 && s[n - 1] == ' ')
        n--;
    if (n == 0 || n > max)
        return GV_EBADARG;
    for (i = 0; i < n; i++) {
        if (s[i] == ' ')
            return GV_EBADARG;
        out[i] = (char)toupper((unsigned char)s[i]);
    }
    out[n] = '\0';
    return GV_OK;
}

#define NORMGN(group, name, g, nm)                                       \
    char g[GV_GRPLEN + 1], nm[GV_NAMLEN + 1];                            \
    if (norm(group, g, GV_GRPLEN, "UNNAMED") || norm(name, nm, GV_NAMLEN, NULL)) \
        return GV_EBADARG

/* ---------------------------------------------------------------- */
/* the table                                                        */
/* ---------------------------------------------------------------- */

static GVENT *find(GV_HANDLE *h, const char *grp, const char *name)
{
    int i;
    for (i = 0; i < h->n; i++)
        if (strcmp(h->ent[i].grp, grp) == 0 &&
            strcmp(h->ent[i].name, name) == 0)
            return &h->ent[i];
    return NULL;
}

static void drop(GV_HANDLE *h, GVENT *e)
{
    h->dirty |= e->life;
    h->changed = 1;
    memmove(e, e + 1, (size_t)((h->ent + h->n) - (e + 1)) * sizeof(GVENT));
    h->n--;
}

static GVENT *add(GV_HANDLE *h)
{
    if (h->n >= GV_MAXENT)
        return NULL;
    if (h->n == h->cap) {
        int    cap = h->cap ? h->cap * 2 : 64;
        GVENT *ne;
        if (cap > GV_MAXENT)
            cap = GV_MAXENT;
        ne = (GVENT *)realloc(h->ent, (size_t)cap * sizeof(GVENT));
        if (ne == NULL)
            return NULL;
        h->ent = ne;
        h->cap = cap;
    }
    memset(&h->ent[h->n], 0, sizeof(GVENT));
    return &h->ent[h->n++];
}

/* ---------------------------------------------------------------- */
/* the stores                                                       */
/* ---------------------------------------------------------------- */

static void torec(const GVENT *e, unsigned char flags, char *r)
{
    int n = (int)strlen(e->grp);
    memset(r, 0, GV_RECLEN);
    memset(r, ' ', GV_GRPLEN);
    memcpy(r, e->grp, (size_t)n);
    r[8] = (char)(F_USED | flags);
    n = (int)strlen(e->name);
    r[9] = (char)n;
    memset(r + 10, ' ', GV_NAMLEN);
    memcpy(r + 10, e->name, (size_t)n);
    r[42] = (char)e->vlen;
    memset(r + 43, ' ', GV_VALLEN);
    memcpy(r + 43, e->val, (size_t)e->vlen);
}

static int fromrec(GV_HANDLE *h, const char *r, unsigned char keep)
{
    GVENT *e;
    int    nl = (unsigned char)r[9], vl = (unsigned char)r[42], gl = GV_GRPLEN;
    if (!((unsigned char)r[8] & F_USED) || nl < 1 || nl > GV_NAMLEN || vl < 1)
        return 1;
    while (gl > 0 && r[gl - 1] == ' ')
        gl--;
    if (gl == 0)
        return 1;
    e = add(h);
    if (e == NULL)
        return GV_EFULL;
    memcpy(e->grp, r, (size_t)gl);
    e->grp[gl] = '\0';
    memcpy(e->name, r + 10, (size_t)nl);
    e->name[nl] = '\0';
    memcpy(e->val, r + 43, (size_t)vl);
    e->val[vl] = '\0';
    e->vlen = vl;
    e->life = (unsigned char)((unsigned char)r[8] & keep);
    return 0;
}

/* 0 loaded, 1 no such store */
static int loadstore(GV_HANDLE *h, int store, unsigned char keep)
{
    char rec[GV_RECLEN];
    int  rc = gvs_ropen(store);
    if (rc != 0)
        return 1;
    while ((rc = gvs_read(rec)) > 0)
        if (fromrec(h, rec, keep) == GV_EFULL)
            break;
    gvs_rclose();
    return 0;
}

static int savestore(GV_HANDLE *h, int store, unsigned char bit)
{
    char rec[GV_RECLEN];
    int  i, any = 0;
    for (i = 0; i < h->n; i++)
        if (bit == 0 || (h->ent[i].life & bit))
            any = 1;
    if (!any)
        return gvs_erase(store);
    if (gvs_wopen(store) != 0)
        return 1;
    for (i = 0; i < h->n; i++)
        if (bit == 0 || (h->ent[i].life & bit)) {
            torec(&h->ent[i], bit == 0 ? h->ent[i].life
                                       : (unsigned char)(h->ent[i].life & bit), rec);
            if (gvs_write(rec) != 0) {
                gvs_wclose();
                return 1;
            }
        }
    return gvs_wclose();
}

/* ---------------------------------------------------------------- */
/* handle calls                                                     */
/* ---------------------------------------------------------------- */

int gv_hinit(GV_HANDLE *h)
{
    h->n = 0;
    gvs_erase(GVS_SESSION);
    loadstore(h, GVS_LASTING, GV_LIFE_LASTING);
    h->changed = 1;
    return GV_OK;
}

int gv_open(GV_HANDLE **hp)
{
    GV_HANDLE *h = (GV_HANDLE *)calloc(1, sizeof *h);
    *hp = h;
    if (h == NULL)
        return GV_ENOMEM;
    if (gvs_lock() != 0) {
        free(h);
        *hp = NULL;
        return GV_ESTORE;
    }
    /* The table from the last call; none: the first call (INIT). */
    if (loadstore(h, GVS_TABLE, GV_LIFE_BOTH) != 0)
        gv_hinit(h);
    return GV_OK;
}

void gv_abandon(GV_HANDLE *h)
{
    if (h != NULL) {
        free(h->ent);
        free(h);
        gvs_unlock();
    }
}

int gv_close(GV_HANDLE *h)
{
    int rc = GV_OK;
    if (h == NULL)
        return GV_OK;
    if (h->changed && savestore(h, GVS_TABLE, 0) != 0)
        rc = GV_ESTORE;
    if ((h->dirty & GV_LIFE_SESSION) && savestore(h, GVS_SESSION, GV_LIFE_SESSION) != 0)
        rc = GV_ESTORE;
    if ((h->dirty & GV_LIFE_LASTING) && savestore(h, GVS_LASTING, GV_LIFE_LASTING) != 0)
        rc = GV_ESTORE;
    gv_abandon(h);
    return rc;
}

int gv_hset(GV_HANDLE *h, const char *group, const char *name,
            const char *value, int len, int life)
{
    GVENT *e;
    NORMGN(group, name, g, nm);
    if (len < 0 || len > GV_VALLEN || (len > 0 && value == NULL) ||
        life < GV_LIFE_CALL || life > GV_LIFE_BOTH)
        return GV_EBADARG;
    e = find(h, g, nm);
    if (len == 0) {                     /* a null value drops it */
        if (e != NULL)
            drop(h, e);
        return GV_OK;
    }
    if (e == NULL) {
        e = add(h);
        if (e == NULL)
            return h->n >= GV_MAXENT ? GV_EFULL : GV_ENOMEM;
        strcpy(e->grp, g);
        strcpy(e->name, nm);
    } else {
        h->dirty |= e->life;            /* the files it was in change   */
    }
    e->life = (unsigned char)life;
    h->dirty |= (unsigned char)life;
    memcpy(e->val, value, (size_t)len);
    e->val[len] = '\0';
    e->vlen = len;
    h->changed = 1;
    return GV_OK;
}

int gv_hget(GV_HANDLE *h, const char *group, const char *name,
            char *buf, int size, int *len)
{
    GVENT *e;
    int    n;
    NORMGN(group, name, g, nm);
    e = find(h, g, nm);
    if (len != NULL)
        *len = e ? e->vlen : 0;
    if (buf != NULL && size > 0) {
        n = e ? e->vlen : 0;
        if (n > size - 1)
            n = size - 1;
        if (n > 0)
            memcpy(buf, e->val, (size_t)n);
        buf[n] = '\0';
    }
    return e ? GV_OK : GV_NOTFOUND;
}

int gv_hdrop(GV_HANDLE *h, const char *group, const char *name)
{
    GVENT *e;
    NORMGN(group, name, g, nm);
    e = find(h, g, nm);
    if (e == NULL)
        return GV_NOTFOUND;
    drop(h, e);
    return GV_OK;
}

int gv_hpurge(GV_HANDLE *h, const char *group)
{
    char g[GV_GRPLEN + 1];
    int  i = 0;
    if (norm(group, g, GV_GRPLEN, "UNNAMED"))
        return GV_EBADARG;
    while (i < h->n)
        if (strcmp(h->ent[i].grp, g) == 0)
            drop(h, &h->ent[i]);
        else
            i++;
    return GV_OK;
}

int gv_hnext(GV_HANDLE *h, const char *group, int *pos, GV_VAR *v)
{
    char g[GV_GRPLEN + 1];
    if (group != NULL && norm(group, g, GV_GRPLEN, "UNNAMED"))
        return GV_EBADARG;
    for (; *pos >= 0 && *pos < h->n; (*pos)++) {
        const GVENT *e = &h->ent[*pos];
        if (group != NULL && strcmp(e->grp, g) != 0)
            continue;
        strcpy(v->group, e->grp);
        strcpy(v->name, e->name);
        memcpy(v->value, e->val, (size_t)e->vlen + 1);
        v->vlen = e->vlen;
        v->life = e->life;
        (*pos)++;
        return GV_OK;
    }
    return GV_NOTFOUND;
}

int gv_hgroups(GV_HANDLE *h, int *pos, char *grp)
{
    int j;
    for (; *pos >= 0 && *pos < h->n; (*pos)++) {
        for (j = 0; j < *pos; j++)
            if (strcmp(h->ent[j].grp, h->ent[*pos].grp) == 0)
                break;
        if (j == *pos) {
            strcpy(grp, h->ent[*pos].grp);
            (*pos)++;
            return GV_OK;
        }
    }
    return GV_NOTFOUND;
}

int gv_hcount(GV_HANDLE *h, const char *group)
{
    char g[GV_GRPLEN + 1];
    int  i, n = 0;
    if (group == NULL)
        return h->n;
    if (norm(group, g, GV_GRPLEN, "UNNAMED"))
        return 0;
    for (i = 0; i < h->n; i++)
        if (strcmp(h->ent[i].grp, g) == 0)
            n++;
    return n;
}

/* ---------------------------------------------------------------- */
/* one-shot calls                                                   */
/* ---------------------------------------------------------------- */

#define ONESHOT(call)                                                    \
    GV_HANDLE *h;                                                        \
    int rc = gv_open(&h), wrc;                                           \
    if (rc != GV_OK)                                                     \
        return rc;                                                       \
    rc = (call);                                                         \
    wrc = gv_close(h);                                                   \
    return rc != GV_OK ? rc : wrc

int gv_setn(const char *group, const char *name, const char *value, int len,
            int life)
{
    ONESHOT(gv_hset(h, group, name, value, len, life));
}

int gv_set(const char *group, const char *name, const char *value, int life)
{
    return gv_setn(group, name, value, value ? (int)strlen(value) : 0, life);
}

int gv_get(const char *group, const char *name, char *buf, int size, int *len)
{
    GV_HANDLE *h;
    int        rc = gv_open(&h);
    if (rc != GV_OK)
        return rc;
    rc = gv_hget(h, group, name, buf, size, len);
    gv_abandon(h);                      /* nothing changed */
    return rc;
}

int gv_exists(const char *group, const char *name)
{
    int rc = gv_get(group, name, NULL, 0, NULL);
    return rc == GV_OK ? 1 : rc == GV_NOTFOUND ? 0 : rc;   /* else the error */
}

int gv_drop(const char *group, const char *name)
{
    ONESHOT(gv_hdrop(h, group, name));
}

int gv_purge(const char *group)
{
    ONESHOT(gv_hpurge(h, group));
}

int gv_init(void)
{
    ONESHOT(gv_hinit(h));
}

const char *gv_version(void)
{
    return "GLOBALV " GV_BUILD " (VM/370plus)";
}

const char *gv_strerror(int rc)
{
    switch (rc) {
    case GV_OK:       return "GLOBALV: OK (0)";
    case GV_NOTFOUND: return "GLOBALV: variable not found (1)";
    case GV_EBADARG:  return "GLOBALV: invalid name, group or value (24)";
    case GV_EFULL:    return "GLOBALV: no room for more variables (28)";
    case GV_ESTORE:   return "GLOBALV: error writing the GLOBALV files (32)";
    case GV_ENOMEM:   return "GLOBALV: out of storage (104)";
    case GV_ENOTAVAIL: return "GLOBALV: not available";
    default:          return "GLOBALV: error";
    }
}
