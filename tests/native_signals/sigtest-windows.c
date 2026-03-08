/*
   test_harness.c

   Win32 test harness: starts test_client.exe, and sends signals looking for expected results

   This is implemented in two layers in this single exec: layer one launched layer 2 (with a
   "++layer2++" argument.

   This is so the second layer is launched in its **own console** (CREATE_NEW_CONSOLE), and
   in its own **process-group** (CREATE_NEW_PROCESS_GROUP). This is so the ctrl-c signal
   is only propagated within layer 2 (the hardness and test target) and does not cause our
   caller to receive it and exit itself. It is a windows thing!

   Additionally, For CTRL_CLOSE_EVENT the client is launched in a new console window
   we send a W_CLOSE message to the console window which triggers the CTRL_CLOSE_EVENT

   Layer 1 redirects Layer 2 output to its own stdout for the caller.
   Redirects the harness’s stdout/stderr to a pipe and forwards everything
   to the wrapper’s stdout so CTest still captures the output.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_BUF_SIZE 4096      // Max stdout capture size
#define READY_STRING "waiting"    // String to wait for in initial output
#define READY_WAIT_TIMEOUT_S 10

static char output_buffer[READ_BUF_SIZE] = {0}; // Buffer for combined stdout
static size_t output_buffer_len = 0; // Keep track of current buffer content length
static char found_ready_string = 0; // Flag for initial "hello" wait
static char found_expected_string = 0; // Flag for final expected substring
static char* expected_substring; // Substring to check *after* signal

// Helper to convert signal names for Windows Console Events
static DWORD signal_name_to_windows(const char* sig_name) {
    if (_stricmp(sig_name, "INT") == 0 || _stricmp(sig_name, "CTRL_C") == 0) return CTRL_C_EVENT;
    if (_stricmp(sig_name, "QUIT") == 0 || _stricmp(sig_name, "BREAK") == 0 || _stricmp(sig_name, "CTRL_BREAK") == 0) return CTRL_BREAK_EVENT;
    if (_stricmp(sig_name, "TERM") == 0 || _stricmp(sig_name, "SIGTERM") == 0) return CTRL_CLOSE_EVENT;
    return (DWORD)-1; // Indicate not a console signal
}

// Sleep
static void sleep_ms(int milliseconds) {
    Sleep(milliseconds);
}

/* Builds the command line from the args
 * Starting from the 'from'th argument (1 base - argv[0] is the program name)
 * and prepending 'prefix1' and 'prefix2' if any
 */
static char* build_windows_command_line(int argc, char* argv[], int from, char* prefix1, char* prefix2) {
    size_t total_len = 0;
    // Calculate the required length
    if (prefix1) total_len += strlen(prefix1) + 3; // 3 = 2 speech marks and a space
    if (prefix2) total_len += strlen(prefix2) + 3;

    for (int i = from; i < argc; ++i) {
        size_t arg_len = strlen(argv[i]);
        // Base length + space + potential quotes
        total_len += arg_len + 3; // Generous estimate for space + quotes
        // A more precise quote check could refine this
    }
    total_len++; // For null terminator

    char* cmdline = (char*)malloc(total_len);
    if (!cmdline) {
        printf("*** ERROR *** Malloc failed for command line buffer\n");
        return NULL;
    }
    cmdline[0] = '\0'; // Start with empty string
    size_t current_pos = 0;

    // Add any prefix
    if (prefix1)
    {
        int needs_quotes = (strchr(prefix1, ' ') != NULL || strchr(prefix1, '\t') != NULL || *prefix1 == '\0'); // Quote if space, tab, or empty
        if (needs_quotes) strcat(cmdline, "\"");
        strcat(cmdline, prefix1);
        if (needs_quotes) strcat(cmdline, "\"");
        strcat(cmdline, " ");
        current_pos =  strlen(cmdline);
    }
    if (prefix2)
    {
        int needs_quotes = (strchr(prefix2, ' ') != NULL || strchr(prefix2, '\t') != NULL || *prefix2 == '\0');
        if (needs_quotes) strcat(cmdline, "\"");
        strcat(cmdline, prefix2);
        if (needs_quotes) strcat(cmdline, "\"");
        strcat(cmdline, " ");
        current_pos =  strlen(cmdline);
    }

    for (int i = from; i < argc; ++i) {
        const char* arg = argv[i];
        size_t arg_len = strlen(arg);
        int needs_quotes = (strchr(arg, ' ') != NULL || strchr(arg, '\t') != NULL || *arg == '\0'); // Quote if space, tab, or empty

        // Add quote if needed
        if (needs_quotes) {
             if (current_pos < total_len - 1) {
                cmdline[current_pos++] = '"';
             } else goto overflow;
        }

        // Copy argument - simplistic approach (doesn't handle internal quotes/backslashes rigorously)
        // For truly robust quoting, see CommandLineToArgvW documentation examples.
        if (current_pos + arg_len < total_len) {
             memcpy(cmdline + current_pos, arg, arg_len);
             current_pos += arg_len;
        } else {
             goto overflow;
        }

        // Add closing quote if needed
        if (needs_quotes) {
            if (current_pos < total_len - 1) {
                 cmdline[current_pos++] = '"';
            } else goto overflow;
        }

        // Add space separator (except for the argument)
        if (i < argc - 1) {
            if (current_pos < total_len - 1) {
                cmdline[current_pos++] = ' ';
            } else goto overflow;
        }
    }

    cmdline[current_pos] = '\0'; // Null terminate
    return cmdline;

overflow:
    printf("*** ERROR *** Command line buffer overflow detected during construction (calculated size: %zu).\n", total_len);
    free(cmdline);
    return NULL;
}

/*
 * Copy everything from pipe to our stdout
 * This is used by the first (wrapper) layer to ensure output from layer 2 (test harness)
 * it sent to the caller.
 */
static void relay(HANDLE hRd)
{
    char  buf[4096];
    DWORD n;
    while (ReadFile(hRd, buf, sizeof buf, &n, NULL) && n)
        fwrite(buf, 1, n, stdout);
}

// This is the main function for the test harness - it just launches layer 2 which launches the test client
static int layer1_main(int argc, char* argv[])
{
    // make a pipe for harness output ──────────────────────────────
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hRd = NULL, hWr = NULL;
    if (!CreatePipe(&hRd, &hWr, &sa, 0))
    {
        fprintf(stderr,"*** INTERNAL ERROR *** Layer 1 CreatePipe failed\n");
        exit(1);
    }
    SetHandleInformation(hRd, HANDLE_FLAG_INHERIT, 0);   /* parent side = non-inherit */

    // launch test_harness.exe ─────────────────────────────────────
    STARTUPINFO         si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags    = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);   /* pass through keyboard (unused) */
    si.hStdOutput = hWr;                              /* redirect to our pipe */
    si.hStdError  = hWr;
    si.wShowWindow = SW_HIDE;                         /* hide the extra window */

    // Create command line = my_program_name (i.e. argv[0)) ++layer2++ {the arguments passed to me}
    const char *cmdLine = build_windows_command_line(argc, argv, 1, argv[0], "++layer2++");

    // Print the command line for debugging
    if (!cmdLine) {
        fprintf(stderr,"*** INTERNAL ERROR *** Failed to build command line for layer 2\n");
        CloseHandle(hRd);
        CloseHandle(hWr);
        exit(1); // Command line build error
    }

    // Launch the test harness layer 2
    DWORD createFlags = CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP;

    if (!CreateProcess(NULL, (LPSTR)cmdLine,
                       NULL, NULL, TRUE,
                       createFlags,
                       NULL, NULL, &si, &pi))
    {
        fprintf(stderr, "*** INTERNAL ERROR *** CreateProcess failed launching layer 2(%lu)\n", GetLastError());
        return 1;
    }
    CloseHandle(hWr);          // wrapper keeps only the read end

    // forward harness output ──────────────────────────────────────
    relay(hRd);
    CloseHandle(hRd);

    // wait & propagate exit code ──────────────────────────────────
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD rc = 0;
    GetExitCodeProcess(pi.hProcess, &rc);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    free((void*)cmdLine);

    return (int)rc;            // CTest sees harness’s return value
}

// Console control handler for Level 2 - masks signals so they do not propagate to the level 2 process
static BOOL WINAPI Level2ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
        return TRUE; // Indicate we handled it. If FALSE, Windows will also try to terminate Level 2
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
        return FALSE; // Let Windows handle any other events
    }
}

// Get the console window handle for a given process ID
HWND GetConsoleWindowFromPid(DWORD pid) {
    HWND hwnd = GetTopWindow(NULL);
    while (hwnd) {
        DWORD winPid;
        GetWindowThreadProcessId(hwnd, &winPid);
        if (winPid == pid) {
            wchar_t className[256];
            GetClassNameW(hwnd, className, 256);
            if (wcscmp(className, L"ConsoleWindowClass") == 0) {
                return hwnd;
            }
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return NULL;
}

// Layer 2 main function - this is the test harness that launches the test client
// and handles signals - isolated from the real caller so signals do not propagate to it.
static int layer2_main(int argc, char* argv[])
{
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hChildStdinRd  = NULL, hChildStdinWr  = NULL;
    HANDLE hChildStdoutRd = NULL, hChildStdoutWr = NULL;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // We are processing the command line = which is what was originally passed to layer 1 (i.e. without the ++layer2++)
    // so arg[0] is the program name, arg[1] is ++layer2++ and ignored, and arg[2] is the signal name, etc.

    // Argument Parsing ---
    if (argc < 6) { // Need at least program, signal, substring, timeout, target
        fprintf(stderr, "Usage: \"%s\" <signal_name> <expected_substring> <timeout_seconds> <target_executable> [target_args...]\n", argv[0]);
        fprintf(stderr, "Example: \"%s\" INT \"Caught SIGINT\" 10 ./my_program --input file.txt\n", argv[0]);
        return 2; // Usage error
    }

    const char* signal_name = argv[2];
    expected_substring = argv[3]; // Substring to check *after* signal
    int final_timeout_seconds = atoi(argv[4]); // Timeout *after* signal
    const char *cmdLine = build_windows_command_line(argc, argv, 5, NULL, NULL);
    if (!cmdLine) {
        fprintf(stderr, "*** ERROR *** Failed to build command line for target executable.\n");
        return 1; // Command line build error
    }
    int test_result = 1; // Default to failure (non-zero exit code)

    output_buffer_len = 0; // Keep track of current buffer content length
    found_ready_string = 0; // Flag for initial "hello" wait
    found_expected_string = 0;

    printf("Signal Tester\n");
    printf(" - Target Executable: %s\n", cmdLine);
    printf(" - Target Arguments:");
    if (argc > 6) {
        for (int i = 6; i < argc; ++i) { printf(" \"%s\"", argv[i]); }
    } else {
        printf(" (none)");
    }
    printf("\n - Signal: %s\n - Expected Substring (pre-signal) : '%s'\n - Expected Substring (post-signal): '%s'\n - Timeout: %ds\n",
           signal_name, READY_STRING, expected_substring, final_timeout_seconds);

    DWORD sig_to_send = signal_name_to_windows(signal_name);
    BOOL isKnownSig = (sig_to_send != (DWORD)-1);

    // 1. Create stdin pipe
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &sa, 0))
    {
        fprintf(stderr, "*** ERROR *** could not create stdin pipe\n");
        return 1;
    }
    // Parent doesn’t need the read end
    if (!SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "*** ERROR *** could not set stdin handle info\n");
        return 1;
    }

    // 2. Create stdout pipe
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &sa, 0))
    {
        fprintf(stderr, "*** ERROR *** could not create stdout pipe\n");
        return 1;
    }
    // Parent doesn’t need the write end
    if (!SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0))
    {
        fprintf(stderr, "*** ERROR *** could not set stdout handle info\n");
        return 1;
    }

    // Register the native Windows console handler
    if (!SetConsoleCtrlHandler(Level2ConsoleCtrlHandler, TRUE)) {
        fprintf(stderr, "*** ERROR *** Could not set console control handler.\n");
        return 1;
    }

    // Set up STARTUPINFO for child process
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags      = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // Hide the console window
    si.hStdInput    = hChildStdinRd;
    si.hStdOutput   = hChildStdoutWr;
    si.hStdError    = hChildStdoutWr;

    // Launch the client
    DWORD creationFlags = 0;
    if (sig_to_send == CTRL_CLOSE_EVENT)
    {
        creationFlags = CREATE_NEW_CONSOLE;
    }
    if (!CreateProcess(
            NULL,
            (void*)cmdLine,             // name of our test client executable
            NULL, NULL,  // process/security attributes
            TRUE,                       // inherit handles
            creationFlags,              // creation flags
            NULL, NULL,                 // environment and current directory
            &si, &pi))
    {
        fprintf(stderr, "*** ERROR *** Error CreateProcess failed (%lu)\n", GetLastError());
        return 1;
    }

    /* Parent no longer needs these handles: */
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);

    free((void*)cmdLine);

    printf(" - Process started (PID: %lu)\n", pi.dwProcessId);

    // High-resolution timer setup
    LARGE_INTEGER freq, startCount;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&startCount);
    double timeoutReady = READY_WAIT_TIMEOUT_S;
    double timeoutFinal = final_timeout_seconds;

    // Read loop
    while (1) {
        // Check elapsed
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        double elapsed = (double)(now.QuadPart - startCount.QuadPart) / (double)freq.QuadPart;

        if (!found_ready_string && elapsed > timeoutReady) {
            fprintf(stderr, "*** ERROR *** Timed out waiting for '%s'\n", READY_STRING);
            test_result = 1;
            break;
        }
        if (found_ready_string && elapsed - timeoutReady > timeoutFinal) {
            fprintf(stderr, "*** ERROR *** Timed out waiting for '%s'\n", expected_substring);
            test_result = 1;
            break;
        }
        // Read available data
        DWORD avail = 0;
        if (PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &avail, NULL) && avail > 0) {
            DWORD toRead = min(avail, READ_BUF_SIZE - 1 - output_buffer_len);
            DWORD bytesRead = 0;
            if (ReadFile(hChildStdoutRd, output_buffer + output_buffer_len, toRead, &bytesRead, NULL) && bytesRead > 0) {
                output_buffer_len += bytesRead;
                output_buffer[output_buffer_len] = '\0';
                if (!found_ready_string && strstr(output_buffer, READY_STRING)) {
                    printf(" - Found client pre-signal output '%s'\n", READY_STRING);
                    found_ready_string = 1;
                    // Send signal
                    if (isKnownSig)
                    {
                        if (sig_to_send == CTRL_CLOSE_EVENT)
                        {
                            // Special handling for CTRL_CLOSE_EVENT to close the client's console window
                            HWND consoleHwnd = GetConsoleWindowFromPid(pi.dwProcessId);
                            if (consoleHwnd) {
                                PostMessage(consoleHwnd, WM_CLOSE, 0, 0); // triggers CTRL_CLOSE_EVENT
                            } else
                            {
                                fprintf(stderr, "*** ERROR *** Could not find console window.\n");
                                test_result = 1;
                                break;
                            }
                        }
                        else
                        {
                            if (!GenerateConsoleCtrlEvent(sig_to_send, 0))
                            {
                                fprintf(stderr, "*** ERROR *** GenerateConsoleCtrlEvent failed (%lu)\n", GetLastError());
                                test_result = 1;
                                break;
                            }
                        }
                        printf(" - Sent signal %s\n", signal_name);
                    } else
                    {
                        if (!TerminateProcess(pi.hProcess, 1)) {
                            fprintf(stderr, "*** ERROR *** TerminateProcess failed: %lu\n", GetLastError());
                            test_result = 1;
                            break;
                        }
                        printf(" - Terminated process for unknown signal %s\n", signal_name);
                    }
                }
                if (found_ready_string && !found_expected_string && strstr(output_buffer, expected_substring)) {
                    printf(" - Found expected output '%s'\n", expected_substring);
                    found_expected_string = 1;
                    test_result = 0;
                }
            }
        }

        // Check if the child has exited
        DWORD wait = WaitForSingleObject(pi.hProcess, 0);

        if (wait == WAIT_OBJECT_0) {
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            // Final read
            while (PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &avail, NULL) && avail > 0) {
                DWORD bytesRead = 0;
                DWORD toRead = min(avail, READ_BUF_SIZE - 1 - output_buffer_len);
                ReadFile(hChildStdoutRd, output_buffer + output_buffer_len, toRead, &bytesRead, NULL);
                output_buffer_len += bytesRead;
                output_buffer[output_buffer_len] = '\0';
            }
            if (found_expected_string) {
                printf(" - Child exit code: %lu\n", exitCode);
                test_result = 0;
            } else {
                fprintf(stderr, "*** ERROR *** Expected substring '%s' not found before exit\n", expected_substring);
                printf(" - Captured STDOUT\n-----------------------------\n%s\n-----------------------------\n", output_buffer);
                test_result = 1;
            }
            break;
        }

        sleep_ms(20);
    }

    // Cleanup
    CloseHandle(hChildStdoutRd);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    printf(" - Signal Tester Finished\n");
    if (test_result)
    {
        printf(" -  ***ERROR*** Captured STDOUT\n-----------------------------\n%s\n-----------------------------\n", output_buffer);
    }
    return test_result;
}

/* real main */
int main(int argc, char* argv[])
{
    // Check if the frst argument is "++layer2++"
    if (argc>1 && strcmp(argv[1],"++layer2++") == 0) {
        // Layer 2
        return layer2_main(argc, argv);
    }
    else {
        // Layer 1
        return layer1_main(argc, argv);
    }
}
