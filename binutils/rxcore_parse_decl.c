/* cREXX License (MIT). Shared immutable mandatory-core PARSE declarations.
 * The compiler and linker use the runtime owner's DECL_ONLY table. Runtime
 * bodies and initialization never enter this declaration object. */
#define DECL_ONLY 1
#include "../interpreter/rxvmparse_cursor.c"
