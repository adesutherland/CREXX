/* Private POSIX child-signalling implementation shared with lifecycle tests. */
#ifndef CREXX_RXSPAWN_POSIX_H
#define CREXX_RXSPAWN_POSIX_H

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int rxspawn_signal_unreaped_child(
        pid_t child_pid, int group_owned, int signal_number) {
    int result;

    if (child_pid <= 0) return 0;
    result = kill(group_owned ? -child_pid : child_pid, signal_number);
    if (result == -1 && errno == ESRCH && group_owned) {
        result = kill(child_pid, signal_number);
    }
    if (result == -1 && errno == ESRCH) return 0;
    if (result == -1 && errno == EPERM && group_owned) {
        int saved_errno = errno;
        int observed;
        siginfo_t info = {0};

        /* Darwin skips zombie members when signalling a group and can return
         * EPERM after redirect shutdown lets the child exit. Accept only a
         * confirmed exit of our still-owned child. WNOWAIT keeps its PID
         * reserved for the caller's waitpid and subsequent group cleanup.
         * A live child, failed observation or real permission error remains
         * a failure; never treat EPERM alone as successful termination. */
        do {
            observed = waitid(P_PID, (id_t)child_pid, &info,
                              WEXITED | WNOHANG | WNOWAIT);
        } while (observed == -1 && errno == EINTR);
        if (observed == 0 && info.si_pid == child_pid &&
                (info.si_code == CLD_EXITED || info.si_code == CLD_KILLED ||
                 info.si_code == CLD_DUMPED)) return 0;
        errno = saved_errno;
    }
    return result;
}

#endif
