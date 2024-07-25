//
// Created by Adrian Sutherland on 03/05/2023.
//

#ifdef __linux__
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <pthread.h>
#endif

#ifdef _WIN32
#include <windows.h>
#ifndef _MSC_VER // Windows Visual Studio
#include <stdint.h>
#endif
#endif

#ifdef __APPLE__
#define _GNU_SOURCE            /* See feature_test_macros(7) */
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "rxvmintp.h"
#include "rxvmvars.h"

// Private structure to allow all the threads to share data etc. and
// make the shellspawn() call re-enterent
typedef struct shelldata {
    REDIRECT* pInput;
    REDIRECT* pOutput;
    REDIRECT* pError;
    char *waitThreadErrorText;
    int waitThreadRC;
#ifdef _WIN32
    PROCESS_INFORMATION ChildProcessInfo;
#else
    int ChildProcessPID;
#endif
    int ChildProcessRC;
    char* buffer;
    char* file_path;
    char** argv;
    value* variables;
} SHELLDATA;

// Private structure for output to string thread
struct redirect {
#ifdef _WIN32
    HANDLE hRead;
    HANDLE hWrite;
    HANDLE thread;
#else
    int hRead;
    int hWrite;
    pthread_t thread;
#endif
    char has_thread;
    value* reg;
    int errorCode;
};

// Defined in a header file: typedef struct redirect REDIRECT;

#ifdef _WIN32
#define start_routine LPTHREAD_START_ROUTINE
#define THREAD_RETURN unsigned long
#else
typedef void *(*start_routine)(void *);
#define THREAD_RETURN void*
#endif

// Private functions
static void Error(char *context, char **errorText);
static void CleanUp(SHELLDATA* data);
static int ParseCommand(const char *command_string, char **command, char **file, char ***argv);
static int launchChild(SHELLDATA* data);
static void WaitForProcess(SHELLDATA* data);
static void appendTextOutput(char **outputText, char *inputText);
static void WriteToStdin(REDIRECT* data, char *line, size_t nBytes);
static void redirectInput(value* redirect_reg, value* string_reg, start_routine start);
static void redirectOutput(value* redirect_reg, value* string_reg, start_routine start);
static value* add_new_element(value* array); /* Appends record to an array and returns the new record */
static THREAD_RETURN Output2StringThread(void* lpvThreadParam);
static THREAD_RETURN Output2ArrayThread(void* lpvThreadParam);
static THREAD_RETURN InputFromStringThread(void* lpvThreadParam);
static THREAD_RETURN InputFromArrayThread(void* lpvThreadParam);
#ifndef _WIN32
static int ExeFound(char* exe);
#endif

/* Get Environment Value
 * Sets value (null terminated) (and a handle) from env variable name length name_length (not null terminated)
 * Value can be set to point to a zero length string (if the variable is not set)
 *
 * Returns 1 if value should bee free()d
 * Otherwise returns 0
 */
int getEnvVal(char **value, char *name, size_t name_length) {

    char* nulled_name;
    if (!name_length) {
        *value = "";
        return 0;
    }
    nulled_name = malloc(name_length + 1);
    memcpy(nulled_name, name, name_length);
    nulled_name[name_length] = 0;

#ifdef _WIN32

    wchar_t *wname;
    int wname_length = MultiByteToWideChar(CP_UTF8, 0, nulled_name, -1, NULL, 0);
    wname = (wchar_t *)malloc(wname_length * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, nulled_name, -1, wname, wname_length);

    DWORD len = GetEnvironmentVariableW(wname, NULL, 0);
    if (len > 0) {
        wchar_t *wvalue = (wchar_t *)malloc(len * sizeof(wchar_t));
        GetEnvironmentVariableW(wname, wvalue, len);

        int utf8_length = WideCharToMultiByte(CP_UTF8, 0, wvalue, len, NULL, 0, NULL, NULL);
        *value = malloc(utf8_length + 1);
        WideCharToMultiByte(CP_UTF8, 0, wvalue, len, *value, utf8_length, NULL, NULL);
        (*value)[utf8_length] = '\0';

        free(wvalue);
    }
    else {
        *value = "";
    }
    free(wname);
    free(nulled_name);
    return len > 0 ? 1 : 0;

#else

    *value = getenv(nulled_name);
    if (!(*value)) {
        *value = "";
        free(nulled_name);
    }
    return 0;

#endif
}

/*
 * - A pin, pout or perr does not need to be specified ... in this case the std streams are used.
 * - Command contains the commands string to execute
 * - rc will contain the return code from the command
 * - errorText contains a descriptive text of any error in the spawn
 *   (i.e. NOT from the executed child process). This is set if this returns
 *   a non-zero return code.
 *
 * Return codes
 *  0 - SHELLSPAWN_OK         - All OK
 *  4 - SHELLSPAWN_NOFOUND    - The command was not found
 *  5 - SHELLSPAWN_FAILURE    - Spawn failed unexpectedly (see error text for details)
*/
int shellspawn (const char *command,
                REDIRECT* pIn,
                REDIRECT* pOut,
                REDIRECT* pErr,
                value* variables,
                int *rc,
                char **errorText) {

// Create data structure - and make sure we make all the members empty
    SHELLDATA data;
    data.waitThreadErrorText = 0;
#ifdef _WIN32
    ZeroMemory(&data.ChildProcessInfo, sizeof(PROCESS_INFORMATION));
#else
    data.ChildProcessPID = 0;
#endif
    data.ChildProcessRC = 0;
    data.pInput = pIn;
    data.pOutput = pOut;
    data.pError = pErr;
    data.buffer = 0;
    data.file_path = 0;
    data.argv = 0;
    data.waitThreadRC = 0;
    data.variables = variables;

#ifdef _WIN32
/* Windows does the actual parsing and validating as part of CreateProcess() */
    data.file_path = malloc(strlen(command) + 1);
    strcpy(data.file_path, command);
#else
    // Parse the command
    char *base_name;
    if (ParseCommand(command, &data.buffer, &base_name, &data.argv)) {
        Error("Failure spawn U18", errorText);
        CleanUp(&data);
        return SHELLSPAWN_NOFOUND;
    }

    int commandFound = 0;
    if (ExeFound(base_name)) {
        data.file_path = malloc(sizeof(char) * strlen(base_name) + 1);
        strcpy(data.file_path, base_name);
        commandFound = 1;
    } else if (base_name[0] != '/') {
        // Get PATH environment variable so we can find the exe
        const char *env = getenv("PATH");
        if (env) {
            data.file_path = malloc(sizeof(char) * (strlen(env) + strlen(base_name) + 2)); // Make a buffer big enough
            while (env) {
                char* next_colon = strchr(env, ':');
                size_t length;

                if (next_colon) {
                    length = next_colon - env;
                } else {
                    length = strlen(env);
                }

                strncpy(data.file_path, env, length);
                data.file_path[length] = '\0';

                strcat(data.file_path, "/");
                strcat(data.file_path, base_name);

                if (ExeFound(data.file_path)) {
                    commandFound = 1;
                    break;
                }

                if (next_colon) {
                    env = next_colon + 1;
                } else {
                    env = NULL;
                }
            }
        }
    }

    if (!commandFound) {
        Error("Command not found", errorText);
        CleanUp(&data);
        return SHELLSPAWN_NOFOUND;
    }
#endif

    /* Launch the command */
    int lrc;
    lrc = launchChild(&data);
    if (lrc) {
        CleanUp(&data);
        return lrc;
    }

    /* Wait fot it to complete */
    WaitForProcess(&data);

    // Handle any waitThread errors
    if (data.waitThreadRC) {
        appendTextOutput(errorText,data.waitThreadErrorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }

    *rc = (int) data.ChildProcessRC;

    CleanUp(&data);

    return SHELLSPAWN_OK;
}

/* Create a null redirect pipe */
/* In general,the redirect_reg MUST then be used in shellspawn() to clean up/free memory */
void nullredr(value* redirect_reg) {
    REDIRECT *redirect;
    /* The register has the opaque REDIRECT Structure */
    value_zero(redirect_reg);
    redirect_reg->binary_length = sizeof(REDIRECT);
    redirect_reg->binary_value = malloc(redirect_reg->binary_length);
    redirect = (REDIRECT *) redirect_reg->binary_value;

    redirect->errorCode = 0;
    redirect->reg = 0;
    redirect->has_thread = 0;

#ifdef _WIN32

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Open the NUL device for reading
    redirect->hRead = CreateFile("NUL",
                           GENERIC_READ,
                           0,                  // no sharing
                           &sa,             // set the bInheritHandle flag
                           OPEN_EXISTING, // open existing file only
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);              // no attr. template

    // Open the NUL device for writing
    redirect->hWrite = CreateFile("NUL",
                            GENERIC_WRITE,
                            0,                 // no sharing
                            &sa,            // set the bInheritHandle flag
                            OPEN_EXISTING,// open existing file only
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);              // no attr. template

#else

    redirect->hRead = open("/dev/null", O_RDONLY);
    redirect->hWrite = open("/dev/null", O_WRONLY);

#endif
}

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2str(value* redirect_reg, value* string_reg) {
    redirectOutput(redirect_reg, string_reg, Output2StringThread);
}

/* Create a redirect pipe to string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void redr2arr(value* redirect_reg, value* string_reg) {
    redirectOutput(redirect_reg, string_reg, Output2ArrayThread);
}

/* Create a redirect output pipe */
void redirectOutput(value* redirect_reg, value* string_reg, start_routine start) {

    REDIRECT *redirect;

    /* The register has the opaque REDIRECT Structure */
    value_zero(redirect_reg);
    redirect_reg->binary_length = sizeof(REDIRECT);
    redirect_reg->binary_value = malloc(redirect_reg->binary_length);
    redirect = (REDIRECT*)redirect_reg->binary_value;

    redirect->errorCode = 0;
    redirect->reg = string_reg;
    redirect->has_thread = 0;

#ifdef _WIN32

    redirect->hRead = INVALID_HANDLE_VALUE;
    redirect->hWrite = INVALID_HANDLE_VALUE;
    HANDLE hReadTmp;

    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE}; // Set the bInheritHandle flag: the pipe is inheritable

    // We Create a pipe
    if (!CreatePipe(&hReadTmp, &redirect->hWrite, &sa, 0))
    {
        // Error - try and clean-up
        redirect->errorCode = 1;
        return;
    }

    // Make a non-inheritable duplicate of the reading side of the pipe
    if (!DuplicateHandle(GetCurrentProcess(), hReadTmp,
                         GetCurrentProcess(),
                         &redirect->hRead, // Address of new handle.
                         0, FALSE, // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS))
    {
        // Error - try and clean-up
        CloseHandle(hReadTmp);
        CloseHandle(redirect->hWrite);
        redirect->errorCode = 2;
        return;
    }

    /* We don't want this inheritable handle */
    if (!CloseHandle(hReadTmp))
    {
        // Error - try and clean-up
        CloseHandle(redirect->hRead);
        CloseHandle(redirect->hWrite);
        redirect->errorCode = 3;
        return;
    }
    hReadTmp = NULL;

#else

    int temppipe[2];    // This holds the fd for the input & output of the pipe ([0] for reading, [1] for writing)
    redirect->hRead = -1;
    redirect->hWrite = -1;

    if (pipe(temppipe)) {
        redirect->errorCode = 1;
    }
    redirect->hRead = temppipe[0];
    redirect->hWrite = temppipe[1];

#endif

    // Launch the thread that reads the output
#ifdef _WIN32

    redirect->thread = CreateThread(NULL, 0, start, (LPVOID)redirect, 0, NULL);
    if (redirect->thread == NULL)
    {
        // Error - try and clean-up
        CloseHandle(redirect->hRead);
        CloseHandle(redirect->hWrite);
        redirect->errorCode = 5;
        return;
    }

#else

    if (pthread_create(&(redirect->thread), NULL, start, (void *)redirect)) {
        // Error
        redirect->errorCode = 1;
    }

#endif

    redirect->has_thread = 1;
}

/* Function to handle output to a string */
/* Thread process to handle standard output */
THREAD_RETURN Output2StringThread(void* lpvThreadParam)
{
    REDIRECT* context = (REDIRECT*)lpvThreadParam;
    context->errorCode= 0;
    char lpBuffer[256 + 1]; // Add one for a trailing null if needed

    int reading = 1;

#ifdef _WIN32

    DWORD dwBytesRead;
    while (reading) {
        if (!ReadFile(context->hRead, lpBuffer, 256, &dwBytesRead, NULL)) {
            context->errorCode = 1;
            return 0;
        }
        else if (dwBytesRead == 0) {
            reading = 0;
        }
        string_append_chars(context->reg, lpBuffer, dwBytesRead);
    }

#else

    size_t nBytesRead;
    while (reading) {
        nBytesRead = read(context->hRead, lpBuffer, 256);
        if (nBytesRead == 0) reading = 0;
        else if (nBytesRead == -1) {
            context->errorCode = 1;
            return 0;
        }
        string_append_chars(context->reg, lpBuffer, nBytesRead);
    }

#endif

    return 0;
}

/* Function to handle output to a vector of strings */
THREAD_RETURN Output2ArrayThread(void* lpvThreadParam) {

    REDIRECT* context = (REDIRECT*)lpvThreadParam;

    char lpBuffer[256 + 1]; // Add one for a trailing null if needed
    size_t nBytesRead;
    value *buffer = 0;
    size_t start;
    int reading = 1;
    size_t i;

#ifdef _WIN32
    DWORD dwBytesRead;
    HANDLE hRead = context->hRead;
    while (reading) {
        if (!ReadFile(hRead, lpBuffer, 256, &dwBytesRead, NULL)) {
            context->errorCode = 1;
            return 0;
        }
        else if (dwBytesRead == 0) {
            reading = 0;
        }
        nBytesRead = dwBytesRead;
#else
        int fd = context->hRead;
    while (reading) {
        nBytesRead = read(fd, lpBuffer, 256);
        if (nBytesRead == 0) {
            reading = 0;
        }
        else if (nBytesRead == -1) {
            context->errorCode = 1;
            return 0;
        }
#endif
        start = 0;
        for (i = 0; i < nBytesRead; i++) {
            if (lpBuffer[i] == '\n') {
//                lpBuffer[i] = 0;
                if (!buffer) buffer = add_new_element(context->reg);
#ifdef _WIN32
                /* Remove the \r if it is there */
                if (i > 0 && lpBuffer[i - 1] == '\r')
                    if (i - start - 1) string_append_chars(buffer, lpBuffer + start, i - start - 1);
                else
#endif
                    if (i - start) string_append_chars(buffer, lpBuffer + start, i - start);
                buffer = 0;
                start = i + 1;
            }
        }
        if (start < nBytesRead) {
            if (!buffer) buffer = add_new_element(context->reg);
            string_append_chars(buffer, lpBuffer + start, nBytesRead - start);
        }
    }

    return 0;
}

/* Appends record to an array and returns the new record */
value* add_new_element(value* array) {
    size_t num = array->num_attributes + 1;

    if (num > array->max_num_attributes) {
        /* We need to increase the size of the buffer */
        /* Make the buffer double sized by setting the number of attributes */
        set_num_attributes(array, num * 2);
    }
    /* Set the number of attributes to the requested number */
    set_num_attributes(array, num);

    return array->attributes[num - 1];
}

/* Create a redirect pipe from a string */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void str2redr(value* redirect_reg, value* string_reg) {
    redirectInput(redirect_reg, string_reg, InputFromStringThread);
}

/* Create a redirect pipe from a array */
/* the redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void arr2redr(value* redirect_reg, value* string_reg) {
    redirectInput(redirect_reg, string_reg, InputFromArrayThread);
}

/* Create a redirect pipe from a thread function */
void redirectInput(value* redirect_reg, value* string_reg, start_routine start) {
    REDIRECT *redirect;

    /* The register has the opaque REDIRECT Structure */
    value_zero(redirect_reg);
    redirect_reg->binary_length = sizeof(REDIRECT);
    redirect_reg->binary_value = malloc(redirect_reg->binary_length);
    redirect = (REDIRECT*)redirect_reg->binary_value;
    redirect->errorCode = 0;
    redirect->has_thread = 0;
    redirect->reg = string_reg;

#ifdef _WIN32

    redirect->hRead = INVALID_HANDLE_VALUE;
    redirect->hWrite = INVALID_HANDLE_VALUE;

    HANDLE hWriteTmp;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // We Create a pipe
    if (!CreatePipe(&(redirect->hRead),&hWriteTmp,  &sa, 0))
    {
        // Error - try and clean-up
        redirect->errorCode = 1;
        return;
    }

    // Make a non-inheritable write handle to the pipe (i.e. the parent end)
    if (!DuplicateHandle(GetCurrentProcess(), hWriteTmp,
                         GetCurrentProcess(),
                         &(redirect->hWrite), // Address of new handle.
                         0, FALSE, // Make it uninheritable.
                         DUPLICATE_SAME_ACCESS))
    {
        // Error - try and clean-up
        redirect->errorCode = 2;
        return;
    }

    /* We don't want this closeable handle */
    if (!CloseHandle(hWriteTmp))
    {
        // Error - try and clean-up
        redirect->errorCode = 3;
        return;
    }

    DWORD threadID;
    // Launch the thread that writes to the pipe
    redirect->thread = CreateThread(NULL, 0, start, redirect, 0, &threadID);
    if (redirect->thread == NULL)
    {
        // Error
        redirect->errorCode = 4;
        return;
    }

#else

    int temppipe[2];    // This holds the fd for the input & output of the pipe

    redirect->hRead = -1;
    redirect->hWrite = -1;

    // Create a pipe
    if (pipe(temppipe)) {
        redirect->errorCode = 1;
        return;
    }
    redirect->hRead = temppipe[0];
    redirect->hWrite = temppipe[1];

    // Launch the thread that writes to the pipe
    if (pthread_create(&(redirect->thread), NULL, start, (void *)redirect)) {
        // Error
        redirect->errorCode = 2;
        return;
    }
#endif
    redirect->has_thread = 1;
}

/* Thread process to handle standard input */
THREAD_RETURN InputFromStringThread(void* lpvThreadParam)
{
    REDIRECT* context = (REDIRECT*)lpvThreadParam;
    context->errorCode= 0;

#ifndef _WIN32

    sigset_t signal_mask;

    // Use pthread_sigmask to block the SIG-PIPE (in case we write to the pipe after it was closed by the child process)
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
        context->errorCode = 1;
        return NULL;
    }

#endif

    WriteToStdin(context, context->reg->string_value, context->reg->string_length);
    WriteToStdin(context, "\n", 1);

#ifdef _WIN32
    CloseHandle(context->hWrite);
    context->hWrite = INVALID_HANDLE_VALUE;
#else
    close(context->hWrite);
    context->hWrite = -1;
#endif

    return 0;
}

/* Thread process to handle standard input */
THREAD_RETURN InputFromArrayThread(void* lpvThreadParam)
{
    REDIRECT* context = (REDIRECT*)lpvThreadParam;
    size_t i;
    context->errorCode= 0;

#ifndef _WIN32

    sigset_t signal_mask;

    // Use pthread_sigmask to block the SIG-PIPE (in case we write to the pipe after it was closed by the child process)
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
        context->errorCode = 1;
        return NULL;
    }

#endif

    for (i=0; i<context->reg->num_attributes; i++) {
        WriteToStdin(context, context->reg->attributes[i]->string_value, context->reg->attributes[i]->string_length);
        WriteToStdin(context, "\n", 1);
    }

#ifdef _WIN32
    CloseHandle(context->hWrite);
    context->hWrite = INVALID_HANDLE_VALUE;
#else
    close(context->hWrite);
    context->hWrite = -1;
#endif

    return 0;
}

void WriteToStdin(REDIRECT* data, char *line, size_t nBytes)
{
#ifdef _WIN32
    DWORD nTotalWrote = 0;
    DWORD nBytesWrote = 0;
#else
    size_t nTotalWrote = 0;
    size_t nBytesWrote;
#endif
    while (nTotalWrote < nBytes)
    {

#ifdef _WIN32

        if (!WriteFile(data->hWrite,(line+nTotalWrote),(nBytes-nTotalWrote),&nBytesWrote, NULL))
        {
            if (GetLastError() == ERROR_NO_DATA) {
                // Pipe was closed, a normal exit path - the child exited before processing all input
                return;
            }
            else {
                data->errorCode = 1;
                return;
            }
        }

#else

        nBytesWrote = write(data->hWrite, (void*)(line+nTotalWrote), (nBytes-nTotalWrote));

        if (nBytesWrote == -1)
        {
            if (errno == EPIPE) {
                // Pipe was closed, a normal exit path - the child exited before processing all input
                return;
            }
            else {
                data->errorCode = 1;
                return;
            }
        }

#endif

        nTotalWrote += nBytesWrote;
    }
}

void CleanUp(SHELLDATA* data)
{
    if (data->buffer) {
        free(data->buffer);
        data->buffer = 0;
    }
    if (data->argv) {
        free(data->argv);
        data->argv = 0;
    }
    if (data->file_path) {
        free(data->file_path);
        data->file_path = 0;
    }

#ifdef _WIN32

    PROCESS_INFORMATION* pProcInfo = &(data->ChildProcessInfo);
    if (pProcInfo->hProcess) {
        TerminateProcess(pProcInfo->hProcess, 0);
        CloseHandle(pProcInfo->hProcess);
        pProcInfo->hProcess = NULL;
    }
    if (pProcInfo->hThread) {
        CloseHandle(pProcInfo->hThread);
        pProcInfo->hThread = NULL;
    }

    // Close any pipes
    if (data->pInput && data->pInput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hRead);
        data->pInput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hWrite);
        data->pOutput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hWrite);
        data->pError->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pInput && data->pInput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hWrite);
        data->pInput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hRead);
        data->pOutput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hRead);
        data->pError->hRead = INVALID_HANDLE_VALUE;
    }

    // Close the thread handles in the redirect structures
    if (data->pInput && data->pInput->thread != NULL) {
        CloseHandle(data->pInput->thread);
        data->pInput->thread = NULL;
    }
    if (data->pOutput && data->pOutput->thread != NULL) {
        CloseHandle(data->pOutput->thread);
        data->pOutput->thread = NULL;
    }
    if (data->pError && data->pError->thread != NULL) {
        CloseHandle(data->pError->thread);
        data->pError->thread = NULL;
    }

#else

    if (data->ChildProcessPID) {
        kill(-data->ChildProcessPID,15); // 15=TERM, 9=KILL
        data->ChildProcessPID = 0;
    }

    // Close any pipes
    if (data->pInput && data->pInput->hRead != -1) {
        close(data->pInput->hRead);
        data->pInput->hRead = -1;
    }
    if (data->pOutput && data->pOutput->hWrite != -1) {
        close(data->pOutput->hWrite);
        data->pOutput->hWrite = -1;
    }
    if (data->pError && data->pError->hWrite != -1) {
        close(data->pError->hWrite);
        data->pError->hWrite = -1;
    }
    if (data->pInput && data->pInput->hWrite != -1) {
        close(data->pInput->hWrite);
        data->pInput->hWrite = -1;
    }
    if (data->pOutput && data->pOutput->hRead != -1) {
        close(data->pOutput->hRead);
        data->pOutput->hRead = -1;
    }
    if (data->pError && data->pError->hRead != -1) {
        close(data->pError->hRead);
        data->pError->hRead = -1;
    }
#endif
}

void appendTextOutput(char **outputText, char *inputText) {
    if (*outputText) {
        *outputText = realloc(*outputText, strlen(*outputText) + strlen(inputText) + 1);
        strcat(*outputText, inputText);
    }
    else {
        *outputText = malloc(strlen(inputText) + 1);
        strcpy(*outputText, inputText);
    }
}

// Waits for the child process and all the input/output thread handlers to exit.
void WaitForProcess(SHELLDATA* data)
{

#ifdef _WIN32

    DWORD dwWaitResult;

    // Close the child ends of any pipes
    if (data->pInput && data->pInput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hRead);
        data->pInput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hWrite);
        data->pOutput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hWrite);
        data->pError->hWrite = INVALID_HANDLE_VALUE;
    }

    // Wait for child process to exit
    dwWaitResult = WaitForSingleObject(data->ChildProcessInfo.hProcess, INFINITE);
    if (dwWaitResult == WAIT_OBJECT_0) {
        // The child process has terminated.
        DWORD process_rc;
        if (!GetExitCodeProcess(data->ChildProcessInfo.hProcess, &process_rc)) {
            // Error in GetExitCodeProcess.
            data->waitThreadRC = 1;
            Error("Failure spawn U43", &(data->waitThreadErrorText));
        }
        data->ChildProcessRC = (int)process_rc;
    }
    else {
        // The child process is not signaled.
        data->waitThreadRC = 1;
        Error("Failure spawn U43", &(data->waitThreadErrorText));
    }

    CloseHandle(data->ChildProcessInfo.hProcess);
    data->ChildProcessInfo.hProcess = NULL;
    data->ChildProcessInfo.hThread = NULL;

    // Wait for the Input, Output and Error threads to die
    if (data->pInput && data->pInput->has_thread)
    {
        WaitForSingleObject(data->pInput->thread, INFINITE);
        CloseHandle(data->pInput->thread);
        data->pInput->thread = NULL;
        data->pInput->has_thread = 0;
    }
    if (data->pOutput && data->pOutput->has_thread)
    {
        WaitForSingleObject(data->pOutput->thread, INFINITE);
        CloseHandle(data->pOutput->thread);
        data->pOutput->thread = NULL;
        data->pOutput->has_thread = 0;
    }
    if (data->pError && data->pError->has_thread)
    {
        WaitForSingleObject(data->pError->thread, INFINITE);
        CloseHandle(data->pError->thread);
        data->pError->thread = NULL;
        data->pError->has_thread = 0;
    }

    // Close 'my' end of any pipes
    if (data->pInput && data->pInput->hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pInput->hWrite);
        data->pInput->hWrite = INVALID_HANDLE_VALUE;
    }
    if (data->pOutput && data->pOutput->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pOutput->hRead);
        data->pOutput->hRead = INVALID_HANDLE_VALUE;
    }
    if (data->pError && data->pError->hRead != INVALID_HANDLE_VALUE) {
        CloseHandle(data->pError->hRead);
        data->pError->hRead = INVALID_HANDLE_VALUE;
    }

#else

    pid_t w;
    int status;

    // Close the child ends of any pipes
    if (data->pInput && data->pInput->hRead != -1) {
        close(data->pInput->hRead);
        data->pInput->hRead = -1;
    }
    if (data->pOutput && data->pOutput->hWrite != -1) {
        close(data->pOutput->hWrite);
        data->pOutput->hWrite = -1;
    }
    if (data->pError && data->pError->hWrite != -1) {
        close(data->pError->hWrite);
        data->pError->hWrite = -1;
    }

    // Wait for child process to exit
    int pid;
    pid = data->ChildProcessPID;

    do {
        w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        if (w == -1)
        {
            data->waitThreadRC = 1;
            Error("Failure spawn U43", &data->waitThreadErrorText);
            break;
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    data->ChildProcessPID = 0;

    // Get Return Code
    data->ChildProcessRC = WEXITSTATUS(status);

    /* Wait for the Input, Output and Error threads to die */
    if (data->pInput && data->pInput->has_thread)
    {
        if (pthread_join(data->pInput->thread,NULL)) {
            data->waitThreadRC = 1;
            Error("Failure spawn U44", &data->waitThreadErrorText);
        }
        data->pInput->has_thread = 0;
    }

    if (data->pOutput && data->pOutput->has_thread)
    {
        if (pthread_join(data->pOutput->thread,NULL)) {
            data->waitThreadRC = 1;
            Error("Failure spawn U45", &data->waitThreadErrorText);
        }
        data->pOutput->has_thread = 0;
    }

    if (data->pError && data->pError->has_thread)
    {
        if (pthread_join(data->pError->thread,NULL)) {
            data->waitThreadRC = 1;
            Error("Failure spawn U46", &data->waitThreadErrorText);
        }
        data->pError->has_thread = 0;
    }

    // Close 'my' end of any pipes
    if (data->pInput && data->pInput->hWrite != -1) {
        close(data->pInput->hWrite);
        data->pInput->hWrite = -1;
    }
    if (data->pOutput && data->pOutput->hRead != -1) {
        close(data->pOutput->hRead);
        data->pOutput->hRead = -1;
    }
    if (data->pError && data->pError->hRead != -1) {
        close(data->pError->hRead);
        data->pError->hRead = -1;
    }

#endif

    /* Check for redirect errors */
    if (data->pInput && data->pInput->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U47", &data->waitThreadErrorText);
    }
    if (data->pOutput && data->pOutput->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U48", &data->waitThreadErrorText);
    }
    if (data->pError && data->pError->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U49", &data->waitThreadErrorText);
    }
}

void Error(char *context, char **errorText)
{
    size_t message_len;
    char *message = "%s. Details: RC=%s Text=%s";
    char sRC[10];
    sprintf(sRC, "%d", errno);

    message_len = strlen(message) + strlen((char*)strerror(errno)) + strlen(context) + 11;
    *errorText = malloc(message_len);
    snprintf(*errorText, message_len, context, sRC, (char*)strerror(errno));
}

/* Parse the command to get the arguments */
int ParseCommand(const char *command_string, char **command, char **file, char ***argv) {
    int l = 0;
    int args = 1;
    int a;
    int arg_start;

    *command = malloc(sizeof(char) * (strlen(command_string) + 1));
    if (*command == NULL) {
        *command = 0;
        file = 0;
        *argv = 0;
        return -1;
    }
    strcpy(*command, command_string);

    // Skip Leading Spaces
    for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;

    // Program bin/exe name
    *file = *command + l;
    for (; (*command)[l]; l++) if ((*command)[l] == ' ') break;
    if ((*command)[l] != 0) {
        (*command)[l] = 0;
        l++;
    }

    // Is there any command at all
    if (!file[0]) {
        free(*command);
        *command = 0;
        file = 0;
        *argv = 0;
        return -1;
    }

    // Skip Trailing Spaces
    for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;

    if ((*command)[l] != 0) { // There are some arguments
        arg_start = l;

        // Count Arguments
        while ((*command)[l]) {
            switch ((*command)[l]) {
                case '"':
                    // Read to the end of the string
                    for (l++; (*command)[l]; l++)
                        if ((*command)[l] == '"') {
                            l++;
                            break;
                        }
                    args++;
                    break;
                case '\'':
                    // Read to the end of the string
                    for (l++; (*command)[l]; l++)
                        if ((*command)[l] == '\'') {
                            l++;
                            break;
                        }
                    args++;
                    break;
                default:
                    for (l++; (*command)[l]; l++)
                        if ((*command)[l] == ' ') {
                            l++;
                            break;
                        }
                    args++;
                    break;
            }
            // Skip Trailing Spaces
            for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;
        }
    }

    *argv = malloc(sizeof(char*) * (args + 1));
    if (*argv == NULL) {
        free(*command);
        *command = 0;
        file = 0;
        *argv = 0;
        return -1;
    }
    if (((*argv)[0] = strrchr(*file, '/')) != NULL)
        (*argv)[0]++;
    else
        *argv[0] = *file;

    // Null Terminator
    (*argv)[args] = 0;

    // Process Arguments
    if (args > 1) {
        a = 1;
        l = arg_start;
        while ((*command)[l]) {
            switch ((*command)[l]) {
                case '"':
                    *argv[a] = *command + l + 1;
                    for (l++; (*command)[l]; l++) {
                        if ((*command)[l] == '"') {
                            (*command)[l] = 0;
                            l++;
                            break;
                        }
                    }
                    a++;
                    break;
                case '\'':
                    *argv[a] = *command + l + 1;
                    for (l++; (*command)[l]; l++) {
                        if ((*command)[l] == '\'') {
                            (*command)[l] = 0;
                            l++;
                            break;
                        }
                    }
                    a++;
                    break;
                default:
                    (*argv)[a] = *command + l;
                    for (l++; (*command)[l]; l++) {
                        if ((*command)[l] == ' ') {
                            (*command)[l] = 0;
                            l++;
                            break;
                        }
                    }
                    a++;
                    break;
            }
            // Skip Trailing Spaces
            for (; (*command)[l]; l++) if ((*command)[l] != ' ') break;
        }
    }

    return 0;
}

// Launches the child job - never returns
int launchChild(SHELLDATA* data) {

#ifdef _WIN32

    // Launch the redirected command
    STARTUPINFOW si;
    int i;

    // Set up the start up info struct.
    ZeroMemory(&si, sizeof(STARTUPINFOW));
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_USESTDHANDLES;

    si.hStdOutput = (data->pOutput && data->pOutput->hWrite != INVALID_HANDLE_VALUE) ? data->pOutput->hWrite : GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = (data->pError && data->pError->hWrite != INVALID_HANDLE_VALUE) ? data->pError->hWrite : GetStdHandle(STD_ERROR_HANDLE);
    si.hStdInput = (data->pInput && data->pInput->hRead != INVALID_HANDLE_VALUE) ? data->pInput->hRead : GetStdHandle(STD_INPUT_HANDLE);

    int flags = CREATE_UNICODE_ENVIRONMENT; // UTF16 Environment Variables

    /* Environment variables */
    LPWSTR pszCurrentEnvironment = GetEnvironmentStringsW();  // Get parent process's environment block.
    if (pszCurrentEnvironment == NULL) {
        // Handle error.
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Calculate the size of the parent's environment block.
    LPWSTR pszTemp = pszCurrentEnvironment;
    while (*pszTemp) {
        pszTemp += wcslen(pszTemp) + 1;
    }
    size_t parentEnvironmentSize = pszTemp - pszCurrentEnvironment;

    // Calculate total length of the new environment block.
    size_t newEnvironmentSize = parentEnvironmentSize + 1; // +1 For the final extra '\0'
    for (i = 0; i + 1 < data->variables->num_attributes; i += 2) {
        newEnvironmentSize += MultiByteToWideChar(CP_UTF8, 0,
                                                  data->variables->attributes[i]->string_value,
                                                  (int) data->variables->attributes[i]->string_length, NULL,
                                                  0);
        newEnvironmentSize += MultiByteToWideChar(CP_UTF8, 0,
                                                  data->variables->attributes[i + 1]->string_value,
                                                  (int) data->variables->attributes[i + 1]->string_length, NULL,
                                                  0);
        newEnvironmentSize += 2;  // For the '=' and '\0'.
    }
    newEnvironmentSize++;  // For the final extra '\0'.

    // Allocate the new environment block.
    LPWSTR pszNewEnvironment = (LPWSTR) calloc(newEnvironmentSize, sizeof(wchar_t));
    if (pszNewEnvironment == NULL) {
        // Handle memory allocation failure.
        FreeEnvironmentStringsW(pszCurrentEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Copy the parent's environment into the new environment block.
    memcpy(pszNewEnvironment, pszCurrentEnvironment,
           parentEnvironmentSize * sizeof(wchar_t));

    FreeEnvironmentStringsW(pszCurrentEnvironment);

    // Add the custom variables at the end of the new environment block.
    LPWSTR pszCurrentVariable = pszNewEnvironment + parentEnvironmentSize;

    for (i = 0; i + 1 < data->variables->num_attributes; i += 2) {

        // Assuming string_value is UTF-8 encoded
        int numChars = MultiByteToWideChar(CP_UTF8, 0,
                                           data->variables->attributes[i]->string_value,
                                           (int)data->variables->attributes[i]->string_length,
                                           pszCurrentVariable, (int)(newEnvironmentSize - (pszCurrentVariable - pszNewEnvironment)));

        pszCurrentVariable += numChars;
        *pszCurrentVariable++ = L'=';

        numChars = MultiByteToWideChar(CP_UTF8, 0,
                                       data->variables->attributes[i + 1]->string_value,
                                       (int)data->variables->attributes[i + 1]->string_length,
                                       pszCurrentVariable, (int)(newEnvironmentSize - (pszCurrentVariable - pszNewEnvironment)));

        pszCurrentVariable += numChars;
        *pszCurrentVariable++ = L'\0';
    }

    *pszCurrentVariable++ = L'\0';  // Add the final '\0'.

    /* Make filepath wide too */
    int filePathLength = MultiByteToWideChar(CP_UTF8, 0, data->file_path, -1, NULL, 0);
    if (filePathLength == 0) {
        // Handle the error here. Call GetLastError() to get the error code.
        free(pszNewEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Allocate memory for the wide character string.
    wchar_t* wideFilePath = malloc(filePathLength * sizeof(wchar_t));
    if (wideFilePath == NULL) {
        free(pszNewEnvironment);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    // Do the conversion.
    MultiByteToWideChar(CP_UTF8, 0, data->file_path, -1, wideFilePath, filePathLength);

    /* Start the child process */
    if (!CreateProcessW(NULL,wideFilePath,NULL,NULL,TRUE,
                       flags,pszNewEnvironment,NULL,&si,&data->ChildProcessInfo))
    {
        if (GetLastError() == 2) // File not found
        {
            free(wideFilePath);
            free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_NOFOUND;
        }
        else
        {
            free(wideFilePath);
            free(pszNewEnvironment);
            CleanUp(data);
            return SHELLSPAWN_FAILURE;
        }
    }

    /* Cleanup */
    free(wideFilePath);
    free(pszNewEnvironment);

    return 0;

#else

    if ((data->ChildProcessPID = fork()) == -1) {
        // Error("Failure spawn U33", errorText);
        CleanUp(data);
        return SHELLSPAWN_FAILURE;
    }

    if (data->ChildProcessPID != 0) // Parent Process
        return 0;

    /* Set Environmental Variables */
    int i;
    char *name;
    char *value;
    for (i = 0; i + 1 < data->variables->num_attributes; i += 2) {
        /* Variable Name */
        name = malloc(data->variables->attributes[i]->string_length + 1);
        memcpy(name, data->variables->attributes[i]->string_value, data->variables->attributes[i]->string_length);
        name[data->variables->attributes[i]->string_length] = 0;

        /* Uppercase it - following exported variables convention on posix */
        char *s = name;
        while (*s) {
            *s = (char)toupper(*s);
            s++;
        }

        /* Variable Value */
        value = malloc(data->variables->attributes[i + 1]->string_length + 1);
        memcpy(value, data->variables->attributes[i + 1]->string_value, data->variables->attributes[i + 1]->string_length);
        value[data->variables->attributes[i + 1]->string_length] = 0;

        /* Set/export variable */
        setenv(name, value,1);

        free(value);
        free(name);
    }

    // Close parent end of the pipes
    if (data->pInput && data->pInput->hWrite != -1) {
        close(data->pInput->hWrite);
        data->pInput->hWrite = -1;
    }
    if (data->pOutput && data->pOutput->hRead != -1) {
        close(data->pOutput->hRead);
        data->pOutput->hRead = -1;
    }
    if (data->pError && data->pError->hRead != -1) {
        close(data->pError->hRead);
        data->pError->hRead = -1;
    }

    /* Duplicate to replace standard streams */
    if (data->pInput && data->pInput->hRead != -1) {
        dup2(data->pInput->hRead, 0);
    }
    if (data->pOutput && data->pOutput->hWrite != -1) {
        dup2(data->pOutput->hWrite, 1);
    }
    if (data->pError && data->pError->hWrite != -1) {
        dup2(data->pError->hWrite, 2);
    }

    /* Set the handling for job control signals back to the default. */
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    // Execute the command
    execv(data->file_path, data->argv);
    perror("Failure spawn launchChild");
    exit(-1);
#endif
}

#ifndef _WIN32
int ExeFound(char* exe)
{
    if(access(exe, X_OK) == 0) return 1;
    else return 0;
}
#endif