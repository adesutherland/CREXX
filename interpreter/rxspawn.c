//
// Created by Adrian Sutherland on 03/05/2023.
//

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#ifdef __APPLE__
#include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

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
    int ChildProcessPID;
    int ChildProcessRC;
    char* buffer;
    char* file_path;
    char** argv;
} SHELLDATA;

// Private structure for output to string thread
struct redirect {
    int hRead;
    int hWrite;
    pthread_t thread;
    char has_thread;
    value* reg;
    int errorCode;
};

typedef void *(*start_routine)(void *);

// Private functions
static void Error(char *context, char **errorText);
static void CleanUp(SHELLDATA* data);
static int ParseCommand(const char *command_string, char **command, char **file, char ***argv);
static void launchChild(SHELLDATA* data);
static int ExeFound(char* exe);
static void WaitForProcess(SHELLDATA* data);
static void appendTextOutput(char **outputText, char *inputText);
static void* Output2StringThread(void* lpvThreadParam);
static void* Output2ArrayThread(void* lpvThreadParam);
static void* InputFromStringThread(void* lpvThreadParam);
static void* InputFromArrayThread(void* lpvThreadParam);
static void WriteToStdin(REDIRECT* data, char *line, size_t nBytes);
static void redirectInput(value* redirect_reg, value* string_reg, start_routine start);
static void redirectOutput(value* redirect_reg, value* string_reg, start_routine start);
static value* add_new_element(value* array); /* Appends record to an array and returns the new record */

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
                int *rc,
                char **errorText) {

// Create data structure - and make sure we make all the members empty
    SHELLDATA data;
    data.waitThreadErrorText = 0;
    data.ChildProcessPID = 0;
    data.ChildProcessRC = 0;
    data.pInput = pIn;
    data.pOutput = pOut;
    data.pError = pErr;
    data.buffer = 0;
    data.file_path = 0;
    data.argv = 0;
    data.waitThreadRC = 0;

    // Parse the command
    char *base_name;
    int i;
    int commandFound = 0;
    if (ParseCommand(command, &data.buffer, &base_name, &data.argv)) {
        Error("Failure spawn U18", errorText);
        CleanUp(&data);
        return SHELLSPAWN_NOFOUND;
    }

    if (ExeFound(base_name)) {
        data.file_path = malloc(sizeof(char) * strlen(base_name) + 1);
        strcpy(data.file_path, base_name);
        commandFound = 1;
    } else if (base_name[0] != '/') {
        // Get PATH environment variable so we can find the exe
        const char *env = getenv("PATH");
        if (env) data.file_path = malloc(sizeof(char) * (strlen(env) + strlen(base_name) + 2)); // Make a buffer big enough
        while (env && *env != ':') {
            for (i = 0; (data.file_path[i] = *env); i++, env++) {
                if (*env == ':') {
                    data.file_path[i] = 0;
                    env++;
                    break;
                }
            }

            strcat(data.file_path, "/");
            strcat(data.file_path, base_name);

            if (ExeFound(data.file_path)) {
                commandFound = 1;
                break;
            }
        }
    }

    if (!commandFound) {
        Error("Command not found", errorText);
        CleanUp(&data);
        return SHELLSPAWN_NOFOUND;
    }

    if ((data.ChildProcessPID = fork()) == -1) {
        Error("Failure spawn U33", errorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }
    if (data.ChildProcessPID == 0) // Child Process
    {
        launchChild(&data); /* This never returns */
    }

    // We're the Parent Process ...

    // Close the child ends of any pipes
    if (data.pInput && data.pInput->hRead != -1) {
        close(data.pInput->hRead);
        data.pInput->hRead = -1;
    }
    if (data.pOutput && data.pOutput->hWrite != -1) {
        close(data.pOutput->hWrite);
        data.pOutput->hWrite = -1;
    }
    if (data.pError && data.pError->hWrite != -1) {
        close(data.pError->hWrite);
        data.pError->hWrite = -1;
    }

    WaitForProcess(&data);

    // Handle any waitThread errors
    if (data.waitThreadRC) {
        appendTextOutput(errorText,data.waitThreadErrorText);
        CleanUp(&data);
        return SHELLSPAWN_FAILURE;
    }

    *rc = (int) data.ChildProcessRC;

    // Close 'my' end of any pipes
    if (data.pInput && data.pInput->hWrite != -1) {
        close(data.pInput->hWrite);
        data.pInput->hWrite = -1;
    }
    if (data.pOutput && data.pOutput->hRead != -1) {
        close(data.pOutput->hRead);
        data.pOutput->hRead = -1;
    }
    if (data.pError && data.pError->hRead != -1) {
        close(data.pError->hRead);
        data.pError->hRead = -1;
    }

    CleanUp(&data);

    return SHELLSPAWN_OK;
}

/* Create a null redirect pipe */
/* In general, he redirect_reg MUST then be used in shellspawn() to cleanup/free memory */
void nullredr(value* redirect_reg) {
    REDIRECT *redirect;
    /* The register has the opaque REDIRECT Structure */
    value_zero(redirect_reg);
    redirect_reg->binary_length = sizeof(REDIRECT);
    redirect_reg->binary_value = malloc(redirect_reg->binary_length);
    redirect = (REDIRECT *) redirect_reg->binary_value;

    redirect->errorCode = 0;
    redirect->reg = 0;
    redirect->hRead = open("/dev/null", O_RDONLY);;
    redirect->hWrite = open("/dev/null", O_WRONLY);
    redirect->has_thread = 0;
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

/* Create a redirect outpur pipe */
void redirectOutput(value* redirect_reg, value* string_reg, start_routine start) {

    REDIRECT *redirect;
    int temppipe[2];    // This holds the fd for the input & output of the pipe ([0] for reading, [1] for writing)

    /* The register has the opaque REDIRECT Structure */
    value_zero(redirect_reg);
    redirect_reg->binary_length = sizeof(REDIRECT);
    redirect_reg->binary_value = malloc(redirect_reg->binary_length);
    redirect = (REDIRECT*)redirect_reg->binary_value;

    redirect->errorCode = 0;
    redirect->reg = string_reg;
    redirect->hRead = -1;
    redirect->hWrite = -1;
    redirect->has_thread = 0;

    if (pipe(temppipe)) {
        redirect->errorCode = 1;
    }
    redirect->hRead = temppipe[0];
    redirect->hWrite = temppipe[1];

    // Launch the thread that reads the output
    if (pthread_create(&(redirect->thread), NULL, start, (void *)redirect)) {
        // Error
        redirect->errorCode = 1;
    }
    redirect->has_thread = 1;
}

/* Function to handle output to a string */
/* Thread process to handle standard output */
void* Output2StringThread(void* lpvThreadParam)
{
    REDIRECT* context = (REDIRECT*)lpvThreadParam;
    context->errorCode= 0;
    char lpBuffer[256 + 1]; // Add one for a trailing null if needed
    size_t nBytesRead;
    int reading = 1;

    while (reading) {
        nBytesRead = read(context->hRead, lpBuffer, 256);
        if (nBytesRead == 0) reading = 0;
        else if (nBytesRead == -1) {
            context->errorCode = 1;
            return 0;
        }
        string_append_chars(context->reg, lpBuffer, nBytesRead);
    }
    return 0;
}

/* Function to handle output to a vector of strings */
void* Output2ArrayThread(void* lpvThreadParam) {

    REDIRECT* context = (REDIRECT*)lpvThreadParam;

    char lpBuffer[256 + 1]; // Add one for a trailing null if needed
    size_t nBytesRead;
    value *buffer = 0;
    size_t start;
    int reading = 1;
    size_t i;

    while (reading) {
        nBytesRead = read(context->hRead, lpBuffer, 256);
        if (nBytesRead == 0) reading = 0;
        else if (nBytesRead == -1) {
            context->errorCode = 1;
            return NULL;
        }
        start = 0;
        for (i = 0; i < nBytesRead; i++) {
            if (lpBuffer[i] == '\n') {
                lpBuffer[i] = 0;
                if (!buffer) buffer = add_new_element(context->reg);
                if (i-start) string_append_chars(buffer, lpBuffer + start, i-start);
                buffer = 0;
                start = i + 1;
            }
        }
        if (start<(int)nBytesRead) {
            if (!buffer) buffer = add_new_element(context->reg);
            string_append_chars(buffer, lpBuffer + start, nBytesRead - start);
        }
    }

    return NULL;
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
    int temppipe[2];    // This holds the fd for the input & output of the pipe ([0] for reading, [1] for writing)

    /* The register has the opaque REDIRECT Structure */
    value_zero(redirect_reg);
    redirect_reg->binary_length = sizeof(REDIRECT);
    redirect_reg->binary_value = malloc(redirect_reg->binary_length);
    redirect = (REDIRECT*)redirect_reg->binary_value;

    redirect->errorCode = 0;
    redirect->reg = string_reg;
    redirect->hRead = -1;
    redirect->hWrite = -1;
    redirect->has_thread = 0;

    if (pipe(temppipe)) {
        redirect->errorCode = 1;
    }
    redirect->hRead = temppipe[0];
    redirect->hWrite = temppipe[1];

    // Launch the thread that writes to the pipe
    if (pthread_create(&(redirect->thread), NULL, start, (void *)redirect)) {
        // Error
        redirect->errorCode = 1;
    }
    redirect->has_thread = 1;
}

/* Thread process to handle standard input */
void* InputFromStringThread(void* lpvThreadParam)
{
    sigset_t signal_mask;
    REDIRECT* context = (REDIRECT*)lpvThreadParam;
    context->errorCode= 0;

    // Use pthread_sigmask to block the SIG-PIPE (in case we write to the pipe after it was closed by the child process)
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
        context->errorCode = 1;
        return NULL;
    }

    WriteToStdin(context, context->reg->string_value, context->reg->string_length);
    WriteToStdin(context, "\n", 1);

    close(context->hWrite);

    return NULL;
}

/* Thread process to handle standard input */
void* InputFromArrayThread(void* lpvThreadParam)
{
    sigset_t signal_mask;
    size_t i;
    REDIRECT* context = (REDIRECT*)lpvThreadParam;
    context->errorCode= 0;

    // Use pthread_sigmask to block the SIG-PIPE (in case we write to the pipe after it was closed by the child process)
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL)) {
        context->errorCode = 1;
        return NULL;
    }

    for (i=0; i<context->reg->num_attributes; i++) {
        WriteToStdin(context, context->reg->attributes[i]->string_value, context->reg->attributes[i]->string_length);
        WriteToStdin(context, "\n", 1);
    }

    close(context->hWrite);

    return NULL;
}

void WriteToStdin(REDIRECT* data, char *line, size_t nBytes)
{
    size_t nTotalWrote=0;
    size_t nBytesWrote=0;

    while (nTotalWrote<nBytes)
    {
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
        nTotalWrote += nBytesWrote;
    }
}

void CleanUp(SHELLDATA* data)
{
    if (data->ChildProcessPID) kill(-data->ChildProcessPID,15); // 15=TERM, 9=KILL
    if (data->buffer) free(data->buffer);
    if (data->argv) free(data->argv);
    if (data->file_path) free(data->file_path);

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
    pid_t w;
    int status;

    // Wait for child process to exit
    int pid;
    pid = data->ChildProcessPID;

    do {
        w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
        if (w == -1)
        {
            data->waitThreadRC = 1;
            Error("Failure spawn U43", &data->waitThreadErrorText);
            return;
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
            return;
        }
        data->pInput->has_thread = 0;
    }

    if (data->pOutput && data->pOutput->has_thread)
    {
        if (pthread_join(data->pOutput->thread,NULL)) {
            data->waitThreadRC = 1;
            Error("Failure spawn U45", &data->waitThreadErrorText);
            return;
        }
        data->pOutput->has_thread = 0;
    }

    if (data->pError && data->pError->has_thread)
    {
        if (pthread_join(data->pError->thread,NULL)) {
            data->waitThreadRC = 1;
            Error("Failure spawn U46", &data->waitThreadErrorText);
            return;
        }
        data->pError->has_thread = 0;
    }

    /* Check for redirect errors */
    if (data->pInput && data->pInput->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U47", &data->waitThreadErrorText);
        return;
    }
    if (data->pOutput && data->pOutput->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U48", &data->waitThreadErrorText);
        return;
    }
    if (data->pError && data->pError->errorCode) {
        data->waitThreadRC = 1;
        Error("Failure spawn U49", &data->waitThreadErrorText);
        return;
    }
}

void Error(char *context, char **errorText)
{
    size_t message_len;
    char *message = "%s. Linux details: RC=%s Text=%s";
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
void launchChild(SHELLDATA* data)
{
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
}

// alarm handler doesn't need to do anything
// other than simply exist
static void alarm_handler( int sig ) {}

// stat() with a timeout measured in seconds
// will return -1 with errno set to EINTR should
// it time out
static int stat_try( const char *path, struct stat *s, unsigned int seconds )
{
    struct sigaction newact;
    struct sigaction oldact;

    // make sure they're entirely clear (yes I'm paranoid...)
    memset( &newact, 0, sizeof( newact ) );
    memset( &oldact, 0, sizeof( oldact) );

    sigemptyset( &newact.sa_mask );

    // note that does not have SA_RESTART set, so
    // stat() should be interrupted on a signal
    // (hopefully your libc doesn't restart it...)
    newact.sa_flags = 0;
    newact.sa_handler = alarm_handler;
    sigaction( SIGALRM, &newact, &oldact );

    alarm( seconds );

    // clear errno
    errno = 0;
    int rc = stat( path, s );

    // save the errno value as alarm() and sigaction() might change it
    int saved_errno = errno;

    // clear any alarm and reset the signal handler
    alarm( 0 );
    sigaction( SIGALRM, &oldact, NULL );

    errno = saved_errno;
    return( rc );
}

int ExeFound(char* exe)
{
    // Stat the command to see if it exists
    struct stat stat_p;
    if ( stat_try(exe, &stat_p, 1) ) return 0;
    if (!S_ISREG(stat_p.st_mode)) return 0; // Not a regular file
    if ( !( ((stat_p.st_mode & S_IXUSR) && (stat_p.st_uid==geteuid())) ||
            ((stat_p.st_mode & S_IXGRP) && (stat_p.st_gid==getegid())) ||
            (stat_p.st_mode & S_IXOTH) ) ) return 0; // Does not have exec permission

    return 1;
}