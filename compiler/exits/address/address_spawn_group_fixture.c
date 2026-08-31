/* Verify that a controlled POSIX child owns the process group used for tree
 * cancellation.  Windows uses a Job Object and has no corresponding pgid. */

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

int main(void) {
#ifdef _WIN32
    return 0;
#else
    return getpid() == getpgrp() ? 0 : 1;
#endif
}
