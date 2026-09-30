/* Keep the child exited but unreaped: this is the redirect-stop/kill race. */
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* Also exercise Darwin's EPERM result on Linux and verify live-child errors
 * remain failures. Only this test translation unit substitutes kill(). */
static int forced_errno;
static int group_error_only;
static int test_kill(pid_t pid, int signal_number) {
    if (forced_errno && (!group_error_only || pid < 0)) {
        errno = forced_errno;
        return -1;
    }
    return kill(pid, signal_number);
}
#define kill test_kill
#include "rxspawn_posix.h"
#undef kill

static int observe_exit(pid_t child, siginfo_t *info) {
    int result;
    memset(info, 0, sizeof(*info));
    do {
        result = waitid(P_PID, (id_t)child, info, WEXITED | WNOWAIT);
    } while (result == -1 && errno == EINTR);
    return result;
}

static int check_child(int group_owned, int exits_first, int signal_error,
                       int deny_group_only) {
    int ready[2];
    int status = 0;
    int failed = 0;
    char byte;
    siginfo_t info;
    pid_t child;

    if (pipe(ready) != 0) return 1;
    child = fork();
    if (child == 0) {
        close(ready[0]);
        if (group_owned && setpgid(0, 0) != 0) _exit(2);
        if (write(ready[1], "r", 1) != 1) _exit(3);
        close(ready[1]);
        if (exits_first) _exit(0);
        for (;;) pause();
    }
    close(ready[1]);
    if (child < 0) { close(ready[0]); return 1; }
    if (read(ready[0], &byte, 1) != 1) failed = 1;
    close(ready[0]);

    if (!failed && exits_first &&
            (observe_exit(child, &info) != 0 || info.si_pid != child ||
             info.si_code != CLD_EXITED || info.si_status != 0)) failed = 1;
    if (!failed) {
        int expected_error = signal_error && !deny_group_only &&
                !(signal_error == EPERM && group_owned && exits_first);
        int result;
        forced_errno = signal_error;
        group_error_only = deny_group_only;
        result = rxspawn_signal_unreaped_child(child, group_owned, SIGKILL);
        forced_errno = 0;
        group_error_only = 0;
        if ((expected_error && (result != -1 || errno != signal_error)) ||
                (!expected_error && result != 0)) {
            fprintf(stderr,
                    "FAIL: signal exited=%d group=%d injected=%d: rc=%d errno=%d\n",
                    exits_first, group_owned, signal_error, result, errno);
            failed = 1;
        }
        if (expected_error) (void)kill(child, SIGKILL);
    }
    /* Signalling must not reap the child or release its PID for reuse. */
    if (!failed && (observe_exit(child, &info) != 0 ||
                   info.si_pid != child)) failed = 1;
    if (failed) (void)kill(child, SIGKILL);
    while (waitpid(child, &status, 0) == -1 && errno == EINTR) {}
    if (exits_first) {
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) failed = 1;
    } else if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGKILL) failed = 1;
    return failed;
}

int main(void) {
    int failed = 0;
    failed += check_child(1, 1, 0, 0);
    failed += check_child(1, 0, 0, 0);
    failed += check_child(0, 1, 0, 0);
    failed += check_child(0, 0, 0, 0);
    failed += check_child(1, 1, EPERM, 0);
    failed += check_child(1, 0, EPERM, 0);
    failed += check_child(0, 1, EPERM, 0);
    failed += check_child(1, 1, EINVAL, 0);
    /* A group can omit an exiting member before waitid reports its exit.
     * Force that group-only failure while the child still needs a signal;
     * the real direct-child signal must terminate it without reaping it. */
    failed += check_child(1, 0, EPERM, 1);
    if (failed) return 1;
    puts("PASS: POSIX live and exited child termination retains reap ownership");
    return 0;
}
