/* Installed-header C++ compilation control for the new binding macros. */
#include "crexxpa.h"
#ifndef DECL_ONLY
PROCEDURE(make_cpp) {
    SETINT(RETURN, SETOBJECTTYPE(nullptr, RETURN, "cpp_probe.box"));
    RESETSIGNAL
}
METHODPROCEDURE(read_cpp) { SETINT(RETURN, 1); RESETSIGNAL }
PROCEDURE(match_cpp) { SETINT(RETURN, 1); RESETSIGNAL }
#endif
LOADFUNCS
ADDINTERFACE("cpp_probe.counter");
ADDDEFAULTMETHODPROC(read_cpp, "cpp_probe.counter", "read", ".int", "");
ADDCLASS("cpp_probe.box");
ADDIMPLEMENTS("cpp_probe.box", "cpp_probe.counter");
ADDFACTORYPROC(make_cpp, "cpp_probe.box", ".cpp_probe..box", "");
ADDNAMEDFACTORYPROC(make_cpp, "cpp_probe.box", "from", ".cpp_probe..box", "");
ADDMATCHPROC(match_cpp, "cpp_probe.box", "");
ADDNAMEDMATCHPROC(match_cpp, "cpp_probe.box", "from", "");
ADDMETHODPROC(read_cpp, "cpp_probe.box", "read", ".int", "");
ENDLOADFUNCS
