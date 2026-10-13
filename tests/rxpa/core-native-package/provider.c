/* A model-free native dependency proves the generic core package protocol. */
#include "crexxpa.h"
RXPA_PLUGIN_PROCESS_REENTRANT
extern int core_package_dependency(void);
#ifndef DECL_ONLY
PROCEDURE(value) { SETINT(RETURN, core_package_dependency()); RESETSIGNAL }
#endif
LOADFUNCS
ADDPROC(value, "rxcore_package.value", "b", ".int", "");
ENDLOADFUNCS
