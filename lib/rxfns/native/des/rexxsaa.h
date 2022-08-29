/* Definitions for the SAA REXX API */

#ifdef __STDC__
#define Args(a) a
#else
#define Args(a) ()
#endif

#ifdef INCL_REXXSAA
#define INCL_RXSHV
#define INCL_RXSUBCOM
#define INCL_RXSYSEXIT
#define INCL_RXFUNC
#endif

typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef char *PSZ;
typedef char *PCHAR;
typedef short *PSHORT;
typedef long *PLONG;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef USHORT *PUSHORT;
typedef char *PCH;
typedef unsigned char *PUCHAR;

typedef struct _RXSTRING {
        ULONG strlength;
        char *strptr;
} RXSTRING;
typedef RXSTRING *PRXSTRING;

#define MAKERXSTRING(x,c,l)   ((x).strptr=(c),(x).strlength=(l))
#define RXNULLSTRING(x)       (!(x).strptr)
#define RXSTRLEN(x)           ((x).strptr ? (x).strlength : 0UL)
#define RXSTRPTR(x)           ((x).strptr)
#define RXVALIDSTRING(x)      ((x).strptr && (x).strlength)
#define RXZEROLENSTRING(x)    ((x).strptr && !(x).strlength)
#define RXRESULTLEN           256

#define APIRET ULONG     /* some apps use these to get correct */
#define APIENTRY         /* function linkage */
typedef ULONG (*PFN)();

#define RXCOMMAND         1
#define RXSUBROUTINE      2
#define RXFUNCTION        4

#ifdef INCL_RXSYSEXIT

#define RXCMD             0
#define RXCMDHST          1

#define RXSIO             1
#define RXSIOSAY          1
#define RXSIOTRC          2
#define RXSIOTRD          3
#define RXSIODTR          4

#define RXINI             2
#define RXINIEXT          1

#define RXTER             3
#define RXTEREXT          1

#define RXEXITNUM    4   

#define RXENDLST    100

#define RXEXIT_OK         0
#define RXEXIT_NOTREG    30
#define RXEXIT_NOEMEM  1002
#define RXEXIT_BADTYPE 1003

#define RXEXIT_HANDLED       0
#define RXEXIT_NOT_HANDLED   1
#define RXEXIT_RAISE_ERROR (-1)

typedef struct {
   struct {
      unsigned int rxfcfail:1;
      unsigned int rxfcerr:1;
   } rxcmd_flags;
   char *rxcmd_address;
   unsigned short rxcmd_addressl;
   char *rxcmd_dll;
   unsigned short rxcmd_dll_len;
   RXSTRING rxcmd_command;
   RXSTRING rxcmd_retc;
} RXCMDHST_PARM;

typedef struct {
   RXSTRING rxsio_string;
} RXSIOSAY_PARM;

typedef struct {
   RXSTRING rxsio_string;
} RXSIOTRC_PARM;

typedef struct {
   RXSTRING rxsiotrd_retc;
} RXSIOTRD_PARM;

typedef struct {
   RXSTRING rxsiodtr_retc;
} RXSIODTR_PARM;

typedef union {
   RXCMDHST_PARM cmdhst;
   RXSIOSAY_PARM siosay;
   RXSIOTRC_PARM siotrc;
   RXSIOTRD_PARM siotrd;
   RXSIODTR_PARM siodtr;
} EXIT_PARM;
typedef EXIT_PARM *PEXIT;

typedef struct {
   char *sysexit_name;
   short sysexit_code;
} RXSYSEXIT;
typedef RXSYSEXIT *PRXSYSEXIT;

typedef LONG (RexxExitHandler) Args((LONG,LONG,PEXIT));

ULONG RexxRegisterExitExe Args((PSZ EnvName,RexxExitHandler *EntryPoint,
      PUCHAR UserArea));

ULONG RexxDeregisterExit Args((PSZ EnvName,PSZ ModuleName));

ULONG RexxQueryExit Args((PSZ EnvName,PSZ ModuleName,
      unsigned short *flag, unsigned char *area));

#else  /* INCL_RXSYSEXIT */
typedef void *PRXSYSEXIT;
#endif /* INCL_RXSYSEXIT */

/* Now RXSYSEXIT is defined, we can declare RexxStart... */
long RexxStart Args((long argc, PRXSTRING argv, char *name,
     PRXSTRING instore, PSZ envname, long calltype, PRXSYSEXIT exits,
     short *rc, PRXSTRING result));

long RexxStartProgram Args((char *argv0, long argc, PRXSTRING argv, char *name,
     char *callname, PRXSTRING instore, PSZ envname, long calltype,
     int flags, PRXSYSEXIT exits, short *rc, PRXSTRING result));

#ifdef INCL_RXSHV

#define RXSHV_OK       0x00    /* OK */
#define RXSHV_NEWV     0x01    /* New variable */
#define RXSHV_LVAR     0x02    /* Last variable */
#define RXSHV_TRUNC    0x04    /* Name or value has been truncated */
#define RXSHV_BADN     0x08    /* Invalid name */
#define RXSHV_MEMFL    0x10    /* Out of memory */
#define RXSHV_BADF     0x20    /* Invalid function code */

#define RXSHV_NOAVL    0x90    /* Interface not available */

#define RXSHV_SET      0x00    /* Set direct variable */
#define RXSHV_FETCH    0x01    /* Fetch direct variable */
#define RXSHV_DROPV    0x02    /* Drop direct variable */
#define RXSHV_SYSET    0x03    /* Set symbolic variable */
#define RXSHV_SYFET    0x04    /* Fetch symbolic variable */
#define RXSHV_SYDRO    0x05    /* Drop symbolic variable */
#define RXSHV_NEXTV    0x06    /* Get next variable */
#define RXSHV_PRIV     0x07    /* Get private information */
#define RXSHV_EXIT     0x08    /* Set function exit value */

typedef struct shvnode
{
   struct shvnode *shvnext;
   RXSTRING shvname, shvvalue;
   ULONG shvnamelen, shvvaluelen;
   UCHAR shvcode, shvret;
} SHVBLOCK;
typedef struct shvnode *PSHVBLOCK;

ULONG RexxVariablePool Args((PSHVBLOCK RequestBlockList));

#endif /* INCL_RXSHV */

#ifdef INCL_RXSUBCOM

typedef ULONG RexxSubcomHandler Args((PRXSTRING,PUSHORT,PRXSTRING));

ULONG RexxRegisterSubcomExe Args((PSZ EnvName,
      RexxSubcomHandler *EntryPoint, PUCHAR UserArea));

ULONG RexxDeregisterSubcom Args((PSZ EnvName, PSZ ModuleName));

ULONG RexxQuerySubcom Args((PSZ Envname, PSZ ModuleName,
      PUSHORT flag, PUCHAR UserArea));

#define RXSUBCOM_OK           0
#define RXSUBCOM_DUP         10
#define RXSUBCOM_NOTREG      30
#define RXSUBCOM_NOCANDROP   40
#define RXSUBCOM_BADTYPE   1003
#define RXSUBCOM_NOEMEM    1002

#define RXSUBCOM_ERROR        1
#define RXSUBCOM_FAILURE      2

#endif /* INCL_RXSUBCOM */

#ifdef INCL_RXFUNC

typedef ULONG (RexxFunctionHandler) Args((PSZ name, ULONG argc,
        RXSTRING argv[], PSZ QueueName, PRXSTRING Retstr));

ULONG RexxRegisterFunctionDll Args((PSZ name, PSZ dllname, PSZ entryname));

ULONG RexxRegisterFunctionExe Args((PSZ name,
      RexxFunctionHandler *EntryPoint));

ULONG RexxDeregisterFunction Args((PSZ name));

ULONG RexxQueryFunction Args((PSZ name));

#define RXFUNC_OK         0
#define RXFUNC_DEFINED   10
#define RXFUNC_NOTREG    30
#define RXFUNC_NOMEM     20

#endif /* INCL_RXFUNC */
