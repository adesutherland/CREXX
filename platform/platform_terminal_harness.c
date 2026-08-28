/* Focused POSIX terminal snapshot/restore regression harness. */

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <util.h>
#else
#include <pty.h>
#endif

#include "platform.h"

static int read_byte_with_timeout(int fd, char *result,
                                  int timeout_milliseconds) {
    struct pollfd descriptor;
    int poll_result;
    ssize_t count;

    descriptor.fd = fd;
    descriptor.events = POLLIN;
    descriptor.revents = 0;
    do {
        poll_result = poll(&descriptor, 1u, timeout_milliseconds);
    } while (poll_result < 0 && errno == EINTR);
    if (poll_result <= 0) return -1;
    do {
        count = read(fd, result, 1u);
    } while (count < 0 && errno == EINTR);
    return count == 1 ? 0 : -1;
}

static int wait_child_with_timeout(pid_t child, int *status,
                                   int timeout_milliseconds) {
    struct timespec pause_time;
    int elapsed = 0;

    pause_time.tv_sec = 0;
    pause_time.tv_nsec = 10000000L;
    while (elapsed < timeout_milliseconds) {
        pid_t result = waitpid(child, status, WNOHANG);
        if (result == child) return 0;
        if (result < 0 && errno != EINTR) return -1;
        nanosleep(&pause_time, NULL);
        elapsed += 10;
    }
    return -1;
}

static void child_restore_mode(int result_fd) {
    struct termios original;
    struct termios changed;
    struct termios restored;
    char result = 'F';

    if (tcgetattr(STDIN_FILENO, &original) != 0) goto done;
    platform_term_save();
    changed = original;
    changed.c_lflag ^= ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &changed) != 0) goto done;
    platform_term_restore();
    if (tcgetattr(STDIN_FILENO, &restored) != 0) goto done;
    if ((restored.c_lflag & ECHO) == (original.c_lflag & ECHO)) result = 'R';

done:
    (void)write(result_fd, &result, 1u);
    _exit(result == 'R' ? 0 : 1);
}

static void child_disconnect_mode(int ready_fd, int command_fd) {
    char byte = 'F';

    (void)signal(SIGHUP, SIG_IGN);
    platform_term_save();
    byte = 'Y';
    if (write(ready_fd, &byte, 1u) != 1) _exit(1);
    while (read(command_fd, &byte, 1u) < 0 && errno == EINTR) {
    }
    if (byte != 'G') _exit(1);
    platform_term_restore();
    byte = 'D';
    (void)write(ready_fd, &byte, 1u);
    _exit(0);
}

int main(int argc, char **argv) {
    int to_parent[2];
    int to_child[2];
    int master_fd = -1;
    pid_t child;
    char result = 'F';
    int status = 0;
    int disconnected;

    if (argc != 2 ||
            (strcmp(argv[1], "restore") != 0 &&
             strcmp(argv[1], "disconnect") != 0)) {
        fprintf(stderr, "usage: %s restore|disconnect\n", argv[0]);
        return 2;
    }
    disconnected = strcmp(argv[1], "disconnect") == 0;
    if (pipe(to_parent) != 0 || pipe(to_child) != 0) {
        perror("platform terminal harness: pipe");
        return 1;
    }

    child = forkpty(&master_fd, NULL, NULL, NULL);
    if (child < 0) {
        perror("platform terminal harness: forkpty");
        return 1;
    }
    if (child == 0) {
        close(to_parent[0]);
        close(to_child[1]);
        if (disconnected) {
            child_disconnect_mode(to_parent[1], to_child[0]);
        } else {
            child_restore_mode(to_parent[1]);
        }
    }

    close(to_parent[1]);
    close(to_child[0]);
    if (read_byte_with_timeout(to_parent[0], &result, 3000) != 0) {
        fprintf(stderr,
                "platform terminal harness: child did not become ready\n");
        goto fail;
    }

    if (disconnected) {
        char command = 'G';
        if (result != 'Y') {
            fprintf(stderr,
                    "platform terminal harness: disconnect child setup failed\n");
            goto fail;
        }
        close(master_fd);
        master_fd = -1;
        if (write(to_child[1], &command, 1u) != 1 ||
                read_byte_with_timeout(to_parent[0], &result, 3000) != 0 ||
                result != 'D') {
            fprintf(stderr,
                    "platform terminal harness: disconnected restore did not return cleanly\n");
            goto fail;
        }
    } else if (result != 'R') {
        fprintf(stderr,
                "platform terminal harness: saved attributes were not restored\n");
        goto fail;
    }

    if (wait_child_with_timeout(child, &status, 3000) != 0 ||
            !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr,
                "platform terminal harness: child did not exit successfully\n");
        goto fail;
    }
    if (master_fd >= 0) close(master_fd);
    close(to_parent[0]);
    close(to_child[1]);
    printf("PASS: terminal %s semantics\n", argv[1]);
    return 0;

fail:
    (void)kill(child, SIGKILL);
    if (master_fd >= 0) close(master_fd);
    while (waitpid(child, &status, 0) == -1 && errno == EINTR) {
    }
    close(to_parent[0]);
    close(to_child[1]);
    return 1;
}
