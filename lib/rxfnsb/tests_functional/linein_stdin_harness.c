/*
 * Keep a child VM's stdin open after writing one newline-terminated answer.
 * The child must complete without waiting for another byte from stdin.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

static int run_child(const char *runner, const char *program,
                     char *output, size_t output_size) {
    SECURITY_ATTRIBUTES attributes;
    STARTUPINFOA startup;
    PROCESS_INFORMATION process;
    HANDLE stdin_read = NULL;
    HANDLE stdin_write = NULL;
    HANDLE output_read = NULL;
    HANDLE output_write = NULL;
    DWORD written = 0;
    DWORD exit_code = 1;
    DWORD wait_result;
    size_t command_size;
    char *command = NULL;
    size_t used = 0;
    int result = 1;

    ZeroMemory(&attributes, sizeof(attributes));
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;

    if (!CreatePipe(&stdin_read, &stdin_write, &attributes, 0) ||
            !SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0) ||
            !CreatePipe(&output_read, &output_write, &attributes, 0) ||
            !SetHandleInformation(output_read, HANDLE_FLAG_INHERIT, 0)) {
        fprintf(stderr, "linein stdin harness: pipe setup failed (%lu)\n",
                GetLastError());
        goto cleanup;
    }

    command_size = strlen(runner) + strlen(program) + 8;
    command = (char *)malloc(command_size);
    if (!command) {
        fprintf(stderr, "linein stdin harness: out of memory\n");
        goto cleanup;
    }
    snprintf(command, command_size, "\"%s\" \"%s\"", runner, program);

    ZeroMemory(&startup, sizeof(startup));
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = stdin_read;
    startup.hStdOutput = output_write;
    startup.hStdError = output_write;
    ZeroMemory(&process, sizeof(process));

    if (!CreateProcessA(runner, command, NULL, NULL, TRUE, 0, NULL, NULL,
                        &startup, &process)) {
        fprintf(stderr, "linein stdin harness: CreateProcess failed (%lu)\n",
                GetLastError());
        goto cleanup;
    }

    CloseHandle(stdin_read);
    stdin_read = NULL;
    CloseHandle(output_write);
    output_write = NULL;

    if (!WriteFile(stdin_write, "Ada\n", 4, &written, NULL) || written != 4) {
        fprintf(stderr, "linein stdin harness: input write failed (%lu)\n",
                GetLastError());
        TerminateProcess(process.hProcess, 1);
        WaitForSingleObject(process.hProcess, INFINITE);
        goto process_cleanup;
    }

    wait_result = WaitForSingleObject(process.hProcess, 3000);
    if (wait_result == WAIT_TIMEOUT) {
        fprintf(stderr,
                "linein stdin harness: child waited for input after newline\n");
        TerminateProcess(process.hProcess, 1);
        WaitForSingleObject(process.hProcess, INFINITE);
        goto process_cleanup;
    }
    if (wait_result != WAIT_OBJECT_0 ||
            !GetExitCodeProcess(process.hProcess, &exit_code)) {
        fprintf(stderr, "linein stdin harness: child wait failed (%lu)\n",
                GetLastError());
        goto process_cleanup;
    }

    while (used + 1 < output_size) {
        DWORD count = 0;
        DWORD capacity = (DWORD)(output_size - used - 1);
        if (!ReadFile(output_read, output + used, capacity, &count, NULL) ||
                count == 0) {
            break;
        }
        used += count;
    }
    output[used] = '\0';
    result = exit_code == 0 ? 0 : 1;

process_cleanup:
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

cleanup:
    if (stdin_read) CloseHandle(stdin_read);
    if (stdin_write) CloseHandle(stdin_write);
    if (output_read) CloseHandle(output_read);
    if (output_write) CloseHandle(output_write);
    free(command);
    return result;
}

#else

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static double monotonic_seconds(void) {
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0.0;
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static int run_child(const char *runner, const char *program,
                     char *output, size_t output_size) {
    int stdin_pipe[2];
    int output_pipe[2];
    pid_t child;
    int status = 0;
    int child_done = 0;
    int timed_out = 0;
    size_t used = 0;
    double deadline;
    struct timespec pause_time = {0, 10000000};

    if (pipe(stdin_pipe) != 0 || pipe(output_pipe) != 0) {
        perror("linein stdin harness: pipe");
        return 1;
    }

    child = fork();
    if (child < 0) {
        perror("linein stdin harness: fork");
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);
        return 1;
    }

    if (child == 0) {
        if (dup2(stdin_pipe[0], STDIN_FILENO) < 0 ||
                dup2(output_pipe[1], STDOUT_FILENO) < 0 ||
                dup2(output_pipe[1], STDERR_FILENO) < 0) {
            _exit(126);
        }
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);
        execl(runner, runner, program, (char *)NULL);
        _exit(127);
    }

    close(stdin_pipe[0]);
    close(output_pipe[1]);

    if (write(stdin_pipe[1], "Ada\n", 4) != 4) {
        perror("linein stdin harness: write");
        kill(child, SIGKILL);
        waitpid(child, &status, 0);
        close(stdin_pipe[1]);
        close(output_pipe[0]);
        return 1;
    }

    deadline = monotonic_seconds() + 3.0;
    while (!child_done) {
        pid_t wait_result = waitpid(child, &status, WNOHANG);
        if (wait_result == child) {
            child_done = 1;
        } else if (wait_result < 0 && errno != EINTR) {
            perror("linein stdin harness: waitpid");
            break;
        } else if (monotonic_seconds() >= deadline) {
            timed_out = 1;
            fprintf(stderr,
                    "linein stdin harness: child waited for input after newline\n");
            kill(child, SIGKILL);
            waitpid(child, &status, 0);
            break;
        } else {
            nanosleep(&pause_time, NULL);
        }
    }

    close(stdin_pipe[1]);
    while (used + 1 < output_size) {
        ssize_t count = read(output_pipe[0], output + used,
                             output_size - used - 1);
        if (count > 0) {
            used += (size_t)count;
        } else if (count < 0 && errno == EINTR) {
            continue;
        } else {
            break;
        }
    }
    close(output_pipe[0]);
    output[used] = '\0';

    if (timed_out || !child_done) return 1;
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}

#endif

int main(int argc, char **argv) {
    char output[4096] = {0};
    int result;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <rxvme> <program.rxbin>\n", argv[0]);
        return 2;
    }

    result = run_child(argv[1], argv[2], output, sizeof(output));
    if (output[0] != '\0') fputs(output, stdout);
    if (result != 0) return result;

    if (!strstr(output, "PASS: linein stdin returns after one newline")) {
        fprintf(stderr, "linein stdin harness: expected PASS output missing\n");
        return 1;
    }
    return 0;
}
