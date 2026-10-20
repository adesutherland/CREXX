/* cREXX License (MIT). Mandatory declaration-only native PARSE surface.
 * Keep the runtime bodies and their initializer out of the compiler import
 * closure. A direct reference from rxcpfunc retains this object on every
 * supported linker, including constructorless application ports. */
#define DECL_ONLY 1
#include "../interpreter/rxvmparse_cursor.c"
