/*
 * Run the crexx driver with a real controlling terminal.  Pipes cannot expose
 * POSIX foreground-process-group failures such as SIGTTIN/SIGTTOU.
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif

#define LINEIN_PROMPT "READY: linein stdin prompt"
#define LINEIN_PASS "PASS: linein stdin returns after one newline"
#define OUTPUT_SIZE 16384

static double monotonic_seconds(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0.0;
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static void terminate_child(pid_t child, int master_fd) {
    int status;
    (void)kill(child, SIGKILL);
    if (master_fd >= 0) close(master_fd);
    while (waitpid(child, &status, 0) == -1 && errno == EINTR) {
    }
}

int main(int argc, char **argv) {
    char output[OUTPUT_SIZE];
    size_t used = 0;
    unsigned long timeout_milliseconds;
    char *end = NULL;
    pid_t child;
    int master_fd = -1;
    int flags;
    int status = 0;
    int input_sent = 0;
    double deadline;
    struct timespec pause_time;

    if (argc != 4) {
        fprintf(stderr,
                "usage: %s <crexx-driver> <program> <timeout-milliseconds>\n",
                argv[0]);
        return 2;
    }
    timeout_milliseconds = strtoul(argv[3], &end, 10);
    if (end == argv[3] || *end != '\0' || timeout_milliseconds == 0 ||
            timeout_milliseconds > 60000) {
        fprintf(stderr,
                "linein tty harness: timeout must be 1..60000 ms\n");
        return 2;
    }

    output[0] = '\0';
    pause_time.tv_sec = 0;
    pause_time.tv_nsec = 10000000L;

    child = forkpty(&master_fd, NULL, NULL, NULL);
    if (child < 0) {
        perror("linein tty harness: forkpty");
        return 1;
    }
    if (child == 0) {
        execl(argv[1], argv[1], argv[2], (char *)NULL);
        perror("linein tty harness: exec crexx driver");
        _exit(127);
    }

    flags = fcntl(master_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(master_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("linein tty harness: PTY flags");
        terminate_child(child, master_fd);
        return 1;
    }

    deadline = monotonic_seconds() +
            (double)timeout_milliseconds / 1000.0;
    for (;;) {
        ssize_t count = read(master_fd, output + used,
                             sizeof(output) - used - 1u);
        if (count > 0) {
            used += (size_t)count;
            output[used] = '\0';
        } else if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
                   errno != EINTR && errno != EIO) {
            perror("linein tty harness: PTY read");
            terminate_child(child, master_fd);
            return 1;
        }

        if (!input_sent && strstr(output, LINEIN_PROMPT)) {
            ssize_t written = write(master_fd, "Ada\n", 4u);
            if (written != 4) {
                if (written < 0) perror("linein tty harness: PTY write");
                else fprintf(stderr, "linein tty harness: short PTY write\n");
                terminate_child(child, master_fd);
                return 1;
            }
            input_sent = 1;
        }

        {
            pid_t wait_result = waitpid(child, &status, WNOHANG | WUNTRACED);
            if (wait_result == child) {
                if (WIFSTOPPED(status)) {
                    fprintf(stderr,
                            "linein tty harness: crexx driver stopped by signal %d\n",
                            WSTOPSIG(status));
                    terminate_child(child, master_fd);
                    return 1;
                }
                break;
            }
            if (wait_result < 0 && errno != EINTR) {
                perror("linein tty harness: waitpid");
                terminate_child(child, master_fd);
                return 1;
            }
        }

        if (monotonic_seconds() >= deadline) {
            fprintf(stderr,
                    "linein tty harness: timed out; child or descendant likely lost foreground terminal ownership\n");
            if (used) fputs(output, stderr);
            terminate_child(child, master_fd);
            return 1;
        }
        if (used + 1u == sizeof(output)) {
            fprintf(stderr, "linein tty harness: output buffer exhausted\n");
            terminate_child(child, master_fd);
            return 1;
        }
        nanosleep(&pause_time, NULL);
    }

    for (;;) {
        ssize_t count = read(master_fd, output + used,
                             sizeof(output) - used - 1u);
        if (count > 0) {
            used += (size_t)count;
            output[used] = '\0';
            if (used + 1u == sizeof(output)) break;
            continue;
        }
        if (count < 0 && errno == EINTR) continue;
        break;
    }
    close(master_fd);
    if (used) fputs(output, stdout);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        if (WIFEXITED(status)) {
            fprintf(stderr, "linein tty harness: driver exited with code %d\n",
                    WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "linein tty harness: driver died from signal %d\n",
                    WTERMSIG(status));
        }
        return 1;
    }
    if (!input_sent || !strstr(output, LINEIN_PASS)) {
        fprintf(stderr,
                "linein tty harness: expected prompt/pass conversation missing\n");
        return 1;
    }
    return 0;
}
