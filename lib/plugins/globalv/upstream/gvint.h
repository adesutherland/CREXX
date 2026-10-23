/*
 * GVINT H -- GLOBALV (VM/370plus): what a platform provides.  C89.
 *
 * 1. The stores (gvfs_pc.c, gvfs_cms.c).  Each store is a file of fixed
 *    300-byte records (GV_RECLEN), the layout of the GLOBALV ASSEMBLE
 *    table entry:
 *       0  CL8   group, blank padded
 *       8  X     flags  X'80' in use, X'01' session, X'02' lasting
 *       9  X     name length
 *      10  CL32  name
 *      42  X     value length
 *      43  CL255 value
 *     298  CL2   (reserved, zero)
 *    GVS_LASTING  LASTING GLOBALV   kept until erased
 *    GVS_SESSION  SESSION GLOBALV   until GLOBALV INIT
 *    GVS_TABLE    the whole table, every lifetime, until GLOBALV INIT
 *
 * 2. The command's terminal, stack and variables (GV_IO, gvcmd.c):
 *    a program that runs gv_command passes its own, or gets the default
 *    (gv_io_default in the platform file).
 *
 * Build 2, 19 October 2026.
 */
#ifndef GVINT_H
#define GVINT_H

#include "globalv.h"

#define GV_BUILD    "3"
#define GV_RECLEN   300

#define GVS_LASTING 0
#define GVS_SESSION 1
#define GVS_TABLE   2

/* Read: 0 open, 1 no such file.  gvs_read: 1 a record, 0 end, <0 error */
int  gvs_ropen(int store);
int  gvs_read(char *rec);
void gvs_rclose(void);
/* Write: replaces the store as a whole.  0 OK.                         */
int  gvs_wopen(int store);
int  gvs_write(const char *rec);
int  gvs_wclose(void);
int  gvs_erase(int store);
/* The table is locked from gv_open to gv_close (other processes wait).
 * 0 locked (or no locking on this platform).                           */
int  gvs_lock(void);
void gvs_unlock(void);

typedef struct {
    void (*type)(const char *line);         /* LIST, GRPLIST            */
    void (*error)(const char *line);        /* DMSGLBnnnE               */
    int  (*stack)(const char *s, int len);  /* STACK, one line FIFO     */
    /* fetch: 0 set, 1 not set, <0 no variables (the rc GLOBALV gives)  */
    int  (*fetch)(const char *name, char *buf, int size, int *len);
    int  (*store)(const char *name, const char *val, int len);
} GV_IO;

extern const GV_IO gv_io_default;

/* gv_cmdio: gv_command with its own GV_IO.  (Not gv_command_io: CMS
 * external names are 8 characters, GV_COMMA would clash.)               */
int gv_cmdio(const char *operands, int len, const GV_IO *io);

#endif
