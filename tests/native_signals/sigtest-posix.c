#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>  // For cross-platform sleep/delay basics & time()
#include <stddef.h> // For size_t
#include <errno.h> // Include errno for POSIX error checking
#include <unistd.h>     // fork, execv, pipe, read, close, sleep_ms, kill, getpid, etc.
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include <signal.h>     // kill, SIG* constants
// #include <errno.h>      // errno, EINTR, EAGAIN etc. -> Included above
#include <poll.h>       // For timeout reading pipe (optional, currently using simple sleep)
#include <fcntl.h>      // fcntl, O_NONBLOCK
#ifdef __APPLE__
    #include <util.h> // macOS-specific header for openpty
#else
    #include <pty.h>  // Linux-specific header for openpty
#endif
#include <time.h>       // For clock_gettime, nanosleep
#include <strings.h>    // For strcasecmp (common POSIX extension)

// Helper to convert signal names for POSIX signals
int signal_name_to_posix(const char* sig_name) {
    if (strcasecmp(sig_name, "HUP") == 0) return SIGHUP;
    if (strcasecmp(sig_name, "INT") == 0) return SIGINT;
    if (strcasecmp(sig_name, "QUIT") == 0) return SIGQUIT;
    if (strcasecmp(sig_name, "TERM") == 0) return SIGTERM;
    if (strcasecmp(sig_name, "KILL") == 0) return SIGKILL;
    if (strcasecmp(sig_name, "USR1") == 0) return SIGUSR1;
    if (strcasecmp(sig_name, "USR2") == 0) return SIGUSR2;
    if (strcasecmp(sig_name, "PIPE") == 0) return SIGPIPE;
    if (strcasecmp(sig_name, "ALRM") == 0) return SIGALRM;
    // Add others as needed
    return -1; // Unknown signal
}

#define READ_BUF_SIZE 4096      // Max stdout capture size
#define READY_STRING "waiting"    // String to wait for in initial output

// --- Cross-platform Sleep ---
void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    // Loop until nanosleep completes without EINTR
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

// --- High-Resolution Time Reading ---
double get_monotonic_time() {
    struct timespec ts;
    // CLOCK_MONOTONIC is preferred as it's not affected by system time changes
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
         // Fallback to less precise time if monotonic clock fails
         return (double)time(NULL);
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

// --- Static Variables for simplicity ---
static char output_buffer[READ_BUF_SIZE] = {0}; // Buffer for combined stdout
static size_t output_buffer_len = 0; // Keep track of current buffer content length
static char found_ready_string = 0; // Flag for initial "hello" wait
static char found_expected_string = 0; // Flag for final expected substring
static char* expected_substring; // Substring to check *after* signal

// --- Helper to check buffer for expected substrings ---
// --- returns 1 if the READY_STRING is just found, 0 if not ---
// --- returning 1 only happens once for the first READY_STRING found and is used to trigger the signal ---
int check_for_strings() {
    // Check if the ready string is now present
    if (!found_ready_string && strstr(output_buffer, READY_STRING)) {
        printf(" - Found startup output '%s'\n", READY_STRING);
        found_ready_string = 1;
        return 1; // Found the ready string - return 1 so we can send the signal
    }
    // Check if the expected substring is present
    if (!found_expected_string && found_ready_string && strstr(output_buffer, expected_substring)) {
        printf(" - Found expected output '%s'\n", expected_substring);
        found_expected_string = 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // --- Argument Parsing (Updated Order) ---
    if (argc < 5) { // Need at least program, signal, substring, timeout, target_exe
        fprintf(stderr, "Usage: %s <signal_name> <expected_substring> <timeout_seconds> <target_executable> [target_args...]\n", argv[0]);
        fprintf(stderr, "Example: %s INT \"Caught SIGINT\" 10 ./my_program --input file.txt\n", argv[0]);
        return 2; // Usage error
    }

    const char* signal_name = argv[1];
    expected_substring = argv[2]; // Substring to check *after* signal
    int final_timeout_seconds = atoi(argv[3]); // Timeout *after* signal
    const char* target_executable_path = argv[4]; // The program to test
    // target arguments starts from argv[5]
    int test_result = 1; // Default to failure (non-zero exit code)
    int child_exit_status = 0; // exit status of the child process

    output_buffer_len = 0; // Keep track of current buffer content length
    found_ready_string = 0; // Flag for initial "hello" wait
    found_expected_string = 0;

    printf("Signal Tester\n");
    printf(" - Target Executable: %s\n", target_executable_path);
    printf(" - Target Arguments:");
    if (argc > 5) {
        for (int i = 5; i < argc; ++i) { printf(" \"%s\"", argv[i]); }
    } else {
        printf(" (none)");
    }
    printf("\n - Signal: %s\n - Expected Substring (pre-signal) : '%s'\n - Expected Substring (post-signal): '%s'\n - Timeout: %ds\n",
           signal_name, READY_STRING, expected_substring, final_timeout_seconds);

    int sig_to_send = signal_name_to_posix(signal_name);
    if (sig_to_send == -1) {
        fprintf(stderr, "*** ERROR *** Unknown or unsupported signal name '%s' for POSIX.\n", signal_name);
        return 2; // Unknown signal
    }


    // --- POSIX Implementation ---
    pid_t pid = -1;
    int status = 0; // For waitpid status
    int mfd, sfd;

    // Open a new PTY master/slave
    if (openpty(&mfd, &sfd, NULL, NULL, NULL) == -1) {
        perror("*** ERROR *** openpty() failed");
        return 1;
    }

    pid = fork();

    if (pid == -1) {
        perror("*** Error *** fork() failed");
        close(sfd);
        close(mfd);
        return 1;
    } else if (pid == 0) {
        // --- Child process ---
        // Child: attach PTY slave as stdio
        close(mfd);
        dup2(sfd, STDIN_FILENO);
        dup2(sfd, STDOUT_FILENO);
        dup2(sfd, STDERR_FILENO);
        close(sfd);

        // Build argv array for execv
        int target_argc = argc - 5; // Number of arguments for the target program
        // Allocate space for: target_exe_name + target_args + NULL terminator
        char **argv_for_child = malloc((target_argc + 1 + 1) * sizeof(char *));
        if (!argv_for_child) {
            perror("*** Error *** Child malloc failed for argv");
            exit(EXIT_FAILURE); // Use EXIT_FAILURE from stdlib.h
        }

        argv_for_child[0] = argv[4]; // Target executable path is child's argv[0]
        for (int i = 0; i < target_argc; ++i) {
            argv_for_child[i + 1] = argv[i + 5]; // Copy pointers to target arguments
        }
        argv_for_child[target_argc + 1] = NULL; // Null terminate the argv array

        // Execute the target program using execv
        // execv searches PATH if target_executable_path is not absolute/relative
        execv(target_executable_path, argv_for_child);

        // If execv returns, it means an error occurred
        fprintf(stderr, "*** Error *** execv() failed for '%s': %s\n", target_executable_path, strerror(errno));
        free(argv_for_child); // Free memory before exiting on error
        exit(EXIT_FAILURE); // Exit child process with failure code
    } else {
        // --- Parent process ---
        // Close the child's end of the pipe (write end)
        close(sfd);
        printf(" - Process started (PID: %d)\n", pid);

        // --- Loop reading std out and waiting for the exec to finish ---
        double start_wait_time = get_monotonic_time();
        // Set read pipe to non-blocking for the check loop
        int flags = fcntl(mfd, F_GETFL, 0);
        if (flags == -1 || fcntl(mfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            // bail if fcntl fails
            perror("*** error *** fcntl() failed to set O_NONBLOCK");
            test_result = 1; // Failure
            goto cleanup_posix; // Exit test as failure
        }

        while (get_monotonic_time() - start_wait_time < final_timeout_seconds) {
            // Check if the child exited using waitpid with WNOHANG
            pid_t child_pid_check = waitpid(pid, &child_exit_status, WNOHANG);

            if (child_pid_check == pid) { // Child has exited
                // Attempt to read the remaining output
                ssize_t bytes_read_after_exit;
                while(output_buffer_len < READ_BUF_SIZE -1 &&
                      (bytes_read_after_exit = read(mfd, output_buffer + output_buffer_len, READ_BUF_SIZE -1 - output_buffer_len)) > 0) {
                    output_buffer_len += bytes_read_after_exit;
                      }
                output_buffer[output_buffer_len] = '\0';

                check_for_strings(); // Check for the expected substrings in the output buffer

                if (found_expected_string) {
                    test_result = 0; // Success! Exit code 0 indicates success to CTest
                } else {
                    if (!found_ready_string) {
                        fprintf(stderr, "Error: Process exited without finding '%s' in initial output.\n", READY_STRING);
                    }
                    else {
                        fprintf(stderr, "--- FAILURE: Did not find final expected substring '%s' in combined output.\n", expected_substring);
                    }
                    test_result = 1; // Failure
                    const char* exit_reason = "exited prematurely";
                    if (WIFEXITED(child_exit_status)) {
                        fprintf(stderr, "*** error *** Process %s with code %d while waiting for '%s'.\n", exit_reason, WEXITSTATUS(child_exit_status), READY_STRING);
                    } else if (WIFSIGNALED(child_exit_status)) {
                        fprintf(stderr, "*** error *** Process %s by signal %d while waiting for '%s'.\n", exit_reason, WTERMSIG(child_exit_status), READY_STRING);
                    } else {
                        fprintf(stderr, "*** error *** Process %s (unknown status %d) while waiting for '%s'.\n", exit_reason, child_exit_status, READY_STRING);
                    }
                }
                goto cleanup_posix; // Exit test
            } else if (child_pid_check == -1) { // Error during waitpid check
                 // ECHILD means the process already reaped (shouldn't happen here yet); others are real errors
                 test_result = 1; // Failure
                 if (errno != ECHILD) {
                     perror("*** error *** waitpid() failed during ready wait check");
                 }
                 else {
                     // If ECHILD, the process is gone, treat as premature exit
                     fprintf(stderr, "*** error *** Process already reaped (ECHILD) while waiting for '%s'.\n", READY_STRING);
                 }
                goto cleanup_posix;
            }
            // else child_pid_check == 0, meaning child is still running

            // Try reading from the non-blocking pipe
            ssize_t bytes_read_now = 0;
            if (output_buffer_len < READ_BUF_SIZE - 1) {
                // Read as much as possible up to the buffer limit
                bytes_read_now = read(mfd, output_buffer + output_buffer_len, READ_BUF_SIZE - 1 - output_buffer_len);
            } else {
                // Buffer full before finding ready string - bail
                fprintf(stderr, "*** error *** Output Buffer full\n");
                test_result = 1; // Failure
                goto cleanup_posix;
            }

            if (bytes_read_now > 0) {
                 // Successfully read data
                 output_buffer_len += bytes_read_now;
                 output_buffer[output_buffer_len] = '\0'; // Keep null-terminated
                 if (check_for_strings()) {
                     // Found the ready string
                     // Send signal
                     printf(" - Sending signal %d (%s) to PID %d\n", sig_to_send, signal_name, pid);
                     if (kill(pid, sig_to_send) == -1) {
                         // kill() can fail if the process already died (e.g., race condition)
                         // or if permissions are wrong (unlikely for own child).
                         // Check errno. ESRCH means process doesn't exist.
                         if (errno == ESRCH) {
                             fprintf(stderr, "*** Warning *** kill failed with ESRCH - process %d likely already exited.\n", pid);
                         } else {
                             perror("*** Warning *** kill failed"); // Log other errors but proceed
                         }
                         test_result = 1; // Failure
                         goto cleanup_posix; // Exit test as failure
                     }
                     if (found_expected_string) {
                         // If we found the expected string, we can exit now
                         test_result = 0; // Success! Exit code 0 indicates success to CTest
                     }
                 }
            } else if (bytes_read_now == 0) {
                 // EOF - pipe closed by child process
                 // Ignore - the child should exit (or we have a timeout) in the next loops

            } else { // bytes_read_now == -1
                 // Read failed, check errno
                 if (errno != EAGAIN && errno != EWOULDBLOCK) {
                     // A real read error occurred - bail
                     perror("*** error *** read failed during ready wait");
                     test_result = 1; // Failure
                     goto cleanup_posix; // Exit test as failure
                 }
            }

            // Yield CPU briefly before the next check.
            sleep_ms(20);
        } // End of wait loop

        // Time out waiting for the output
        fprintf(stderr, "*** error *** Test timed out\n");
        test_result = 1; // Failure

cleanup_posix: // Label for cleanup jump
        // Ensure pipe is closed
        if (mfd >= 0) close(mfd); // Close read end in parent
        // Ensure child process is reaped if something went wrong before waitpid
        if (pid > 0) {
            int final_status;
            // Non-blocking check in case it was already reaped or never started properly
            if(waitpid(pid, &final_status, WNOHANG) == 0) {
                 // Still running? Should not happen here. Kill it.
                 kill(pid, SIGKILL);
                 waitpid(pid, &final_status, 0); // Blocking wait after kill
            }
            if (test_result == 0) {
                printf(" - Child exit code: %d\n",  WEXITSTATUS(child_exit_status));
            }
            else {
                printf(" - Captured STDOUT\n%s\n-----------------------------\n", output_buffer);
                printf("*** Error *** sigest failed (Result code: %d)\n", test_result);
            }
        }
        printf(" - Signal Tester Finished\n");
        return test_result; // 0 for success, non-zero for failure
    }
}