//
// System Information Plugin for crexx/pa - Plugin Architecture
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

#ifdef _WIN32
#define wait(ms) Sleep(ms);
#else
#define wait(ms) usleep(ms*1000)
#endif


#define MAX_LINE_LENGTH 1024
#define MAX_BUFFER_LENGTH ((64 * 1024) + 1)
/* ----------------------------------------------------------------------------
 * Pipe Process Structure
 * ----------------------------------------------------------------------------
 */
#ifdef _WIN32
typedef struct {
    PROCESS_INFORMATION pi;
    HANDLE hRead;    // Read from child’s stdout/stderr
    HANDLE hWrite;   // Write to child’s stdin
    int mode;
    int running;
    char status;     // a: active, i: inactive c: cancelled  t: terminated
} ChildProcess;
#else
typedef struct {
    pid_t pid;
    int fd;          // Read from child’s stdout/stderr
    int fd_write;    // Write to child’s stdin
    int mode;
    int running;
} ChildProcess;
#endif

/* ----------------------------------------------------------------------------
 * Split a buffer into lines, store into array
 * ----------------------------------------------------------------------------
 */
int splitBuffer(char *buffer, void *array) {
    char line[MAX_LINE_LENGTH];
    int line_len = 0;
    int lino = GETARRAYHI(array);
    char *ptr = buffer;
    while (*ptr) {
        while (*ptr && *ptr != '\n' && *ptr != '\r' && line_len < MAX_LINE_LENGTH - 1) {
            line[line_len++] = *ptr++;
        }
        if (*ptr == '\n' || *ptr == '\r') {
            line[line_len] = '\0';
            lino++;
            SETARRAYHI(array, lino);
            SETSARRAY(array, lino - 1, line);
            line_len = 0;
            line[0] = '\0';
            if (*ptr == '\r' && *(ptr + 1) == '\n') ++ptr;
            ++ptr;
        }
    }
    if (line_len > 0) {
        line[line_len] = '\0';
        lino++;
        SETARRAYHI(array, lino);
        SETSARRAY(array, lino - 1, line);
    }
    return lino;
}
/* ----------------------------------------------------------------------------
 * Read lines from pipe/fd until EOF
 * ----------------------------------------------------------------------------
 */
#ifdef _WIN32
int process_lines_from_pipe(ChildProcess *proc,HANDLE hRead, void *array, int timeout) {
#else
int process_lines_from_pipe(ChildProcess *out_proc,int fd, void *array, int timeout) {
#endif
    int lino = 0;
    int elapsed=0;
    int interval=50;
    char buffer[MAX_BUFFER_LENGTH];
    for (;;) {
#ifdef _WIN32
        DWORD nRead = 0;
        BOOL bSuccess = ReadFile(hRead, buffer, sizeof(buffer) - 1, &nRead, NULL);
        if (bSuccess && nRead > 0) {
#else
            ssize_t nRead = read(fd, buffer, sizeof(buffer) - 1);
        if (nRead > 0) {
#endif
            buffer[nRead] = '\0';
            lino = splitBuffer(buffer, array);
        } else {  // no more data to read, pipe is empty/EOF
            break;
        }
        elapsed += interval;
        if (timeout > 0 && elapsed >= timeout) {
            proc->status= 't';  // terminated by timeout
            break;
        }
        wait(interval);         // wait the interval to allow the called command to provide more lines
    }
    return lino;
}
/* ----------------------------------------------------------------------------
 * Start Child Process (mode: "r", "w", "rw")
 * ----------------------------------------------------------------------------
 */
int start_child_process(ChildProcess *out_proc,const char *cmd, const char *mode) {
#ifdef _WIN32
    HANDLE hChildStdoutRd = NULL, hChildStdoutWr = NULL;
    HANDLE hChildStdinRd = NULL, hChildStdinWr = NULL;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Output (read)
    if (!mode || strchr(mode, 'r')) {
        if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &sa, 0)) return -1;
        SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);
    }
    // Input (write)
    if (mode && strchr(mode, 'w')) {
        if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &sa, 0)) return -2;
        SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    if (!mode || strchr(mode, 'r')) {
        si.hStdOutput = hChildStdoutWr;
        si.hStdError  = hChildStdoutWr;
    }
    if (mode && strchr(mode, 'w')) {
        si.hStdInput = hChildStdinRd;
    }

    ZeroMemory(&pi, sizeof(pi));
    char cmdline[512];
    strncpy(cmdline, cmd, sizeof(cmdline)-1);
    cmdline[sizeof(cmdline)-1] = 0;

    if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        if (hChildStdoutRd) CloseHandle(hChildStdoutRd);
        if (hChildStdoutWr) CloseHandle(hChildStdoutWr);
        if (hChildStdinRd)  CloseHandle(hChildStdinRd);
        if (hChildStdinWr)  CloseHandle(hChildStdinWr);
        return -3;
    }

    if (!mode || strchr(mode, 'r')) CloseHandle(hChildStdoutWr);
    if (mode && strchr(mode, 'w')) CloseHandle(hChildStdinRd);

    out_proc->pi = pi;
    out_proc->hRead   = (!mode || strchr(mode, 'r')) ? hChildStdoutRd : NULL;
    out_proc->hWrite  = (mode && strchr(mode, 'w')) ? hChildStdinWr : NULL;
    out_proc->running = 1;
    out_proc->status  = 'a';
    return 0;
#else
    int pipefd[2] = {-1, -1};
    int pipefd_write[2] = {-1, -1};
    if (!mode || strchr(mode, 'r')) if (pipe(pipefd) == -1) return -1;
    if (mode && strchr(mode, 'w'))  if (pipe(pipefd_write) == -1) return -2;

    pid_t pid = fork();
    if (pid == -1) {
        if (pipefd[0] != -1) { close(pipefd[0]); close(pipefd[1]); }
        if (pipefd_write[0] != -1) { close(pipefd_write[0]); close(pipefd_write[1]); }
        return -3;
    }
    if (pid == 0) {
        // Child
        if (mode && strchr(mode, 'w')) {
            dup2(pipefd_write[0], STDIN_FILENO);
            close(pipefd_write[0]); close(pipefd_write[1]);
        }
        if (!mode || strchr(mode, 'r')) {
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[0]); close(pipefd[1]);
        }
        execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
        exit(127);
    }
    // Parent
    if (!mode || strchr(mode, 'r')) close(pipefd[1]);
    if (mode && strchr(mode, 'w'))  close(pipefd_write[0]);
    out_proc->pid = pid;
    out_proc->fd = (!mode || strchr(mode, 'r')) ? pipefd[0] : -1;
    out_proc->fd_write = (mode && strchr(mode, 'w')) ? pipefd_write[1] : -1;
    out_proc->running = 1;
    out_proc->status  = 'a';
    return 0;
#endif
}
/* ----------------------------------------------------------------------------
 * Write data to child's stdin
 * ----------------------------------------------------------------------------
 */
int pipesendh(ChildProcess *proc, const char *data, size_t len) {
#ifdef _WIN32
    if (!proc->hWrite) return -1;
    DWORD written = 0;
    BOOL ok = WriteFile(proc->hWrite, data, (DWORD)len, &written, NULL);
    return ok ? (int)written : -2;
#else
    if (proc->fd_write < 0) return -1;
    ssize_t n = write(proc->fd_write, data, len);
    return (n >= 0) ? (int)n : -2;
#endif
}
/* ----------------------------------------------------------------------------
 * Close child's stdin when done writing
 * ----------------------------------------------------------------------------
 */
void pipecloseinputh(ChildProcess *proc) {
#ifdef _WIN32
    if (proc->hWrite) { CloseHandle(proc->hWrite); proc->hWrite = NULL; }
#else
    if (proc->fd_write >= 0) { close(proc->fd_write); proc->fd_write = -1; }
#endif
}

/* ----------------------------------------------------------------------------
 * Cancel running pipe
 * ----------------------------------------------------------------------------
 */
#ifdef _WIN32
int cancel_child_process(ChildProcess *proc) {
    if (proc->running < 0) return 0;    // not available
    if (proc->running) {
        BOOL ok = TerminateProcess(proc->pi.hProcess, 1);
        if (ok) {
            proc->running = -1;    // not longer running
            proc->status  = 'i';   // task inactive
            return 0;
        } else {
            return -12;
        }
    }
    return -8;
}
#else
int cancel_child_process(ChildProcess *proc) {
    if (proc->running) {
        int ok = kill(proc->pid, SIGKILL);
        if (ok == 0) {
            proc->running = 0;
            proc->status  = 'i';
            return 0;
        } else {
            return -12;
        }
    }
    return -8;
}
#endif
/* ----------------------------------------------------------------------------
 * Cleanup all resources
 * ----------------------------------------------------------------------------
 */
void cleanup_child_process(ChildProcess *proc) {
#ifdef _WIN32
    if (proc->hRead)   CloseHandle(proc->hRead);
    if (proc->hWrite)  CloseHandle(proc->hWrite);
    CloseHandle(proc->pi.hProcess);
    CloseHandle(proc->pi.hThread);
#else
    if (proc->fd >= 0)      close(proc->fd);
    if (proc->fd_write >= 0) close(proc->fd_write);
#endif
}
/* ----------------------------------------------------------------------------
 * CREXX stubs (replace with your actual system or remove for plain C)
 * ----------------------------------------------------------------------------
 */
PROCEDURE(pipecreate) {
    char * mode = GETSTRING(ARG0);
    ChildProcess *proc = malloc(sizeof(ChildProcess));
    if (!proc) { RETURNINTX(-8); }
    rxinteger rproc = (rxinteger)proc;
    if(mode[0]=='r' || mode[0]=='R') proc->mode=0;
    else proc->mode=1;
    RETURNINTX(rproc);
    ENDPROC
}


PROCEDURE(pipesend) {
    rxinteger rproc = GETINT(ARG0);
    ChildProcess *proc = (ChildProcess *) rproc;
    char * data = GETSTRING(ARG1);

    // Write data to child's stdin
    int n = pipesendh(proc, data, strlen(data));
    RETURNINTX(n);
    ENDPROC
}


/* ----------------------------------------------------------------------------
 * CREXX PIPSEND Send command to a pipe
 * ----------------------------------------------------------------------------
 */

PROCEDURE(piperun) {
    rxinteger rproc = GETINT(ARG0);
    char *cmd  = GETSTRING(ARG1);
    char *mode = GETSTRING(ARG2);  // new: allow mode as parameter ("r", "w", or "rw")
    ChildProcess *proc = (ChildProcess *) rproc;
    if (start_child_process(proc, cmd, mode) != 0) {
        fprintf(stderr, "Failed to start child process\n");
        RETURNINTX(-8);
    }
    proc->running = 1;
    proc->status  = 'a';
    RETURNINTX(0);
    ENDPROC
}

/* ----------------------------------------------------------------------------
 * CREXX ????? PIPSEND Send command to a pipe
 * ----------------------------------------------------------------------------
 */

PROCEDURE(pipecloseinput) {
    rxinteger rproc = GETINT(ARG0);
    ChildProcess *proc = (ChildProcess *) rproc;
    pipecloseinputh(proc);
    RETURNINTX(0);
    ENDPROC
}
PROCEDURE(pipeget) {
    rxinteger rproc = GETINT(ARG0);
    int lino=0;
    ChildProcess *proc = (ChildProcess *) rproc;
#ifdef _WIN32
    lino+=process_lines_from_pipe(proc,proc->hRead, ARG1,GETINT(ARG2));
#else
    lino+=process_lines_from_pipe(proc,proc->fd, ARG1,GETINT(ARG2));
#endif
    printf("running get %d\n",proc->running);
    if(proc->status=='t')  cancel_child_process(proc);  // task ended by timeout, need to cancel task
    printf("running get %d\n",proc->running);

    RETURNINTX(lino);
    ENDPROC
}
PROCEDURE(pipeclose) {
    rxinteger rproc = GETINT(ARG0);
    ChildProcess *proc = (ChildProcess *) rproc;
    cleanup_child_process(proc);
    free(proc);
    RETURNINTX(0);
    ENDPROC
}
/* ----------------------------------------------------------------------------
 * CREXX PIPEWAIT
 * ----------------------------------------------------------------------------
 */
int wait_child_process(ChildProcess *proc, int block, int *exit_code) {
#ifdef _WIN32
    DWORD result = WaitForSingleObject(proc->pi.hProcess, block ? INFINITE : 0);
    if (result == WAIT_OBJECT_0) {
        DWORD code = 0;
        if (exit_code && GetExitCodeProcess(proc->pi.hProcess, &code)) *exit_code = (int)code;
        return 1; // Exited
    }
    if (result == WAIT_TIMEOUT) {
        if (exit_code) *exit_code = -128; // still running
        return 0; // Still running
    }
    if (exit_code) *exit_code = -256;
    return -1; // Error
#else
    int status = 0;
    pid_t res = waitpid(proc->pid, &status, block ? 0 : WNOHANG);
    if (res == 0) {
        if (exit_code) *exit_code = -128; // still running
        return 0; // Still running
    }
    if (res == proc->pid) {
        if (exit_code) {
            if (WIFEXITED(status)) *exit_code = WEXITSTATUS(status);
            else *exit_code = -512; // Abnormal
        }
        return 1; // Exited
    }
    if (exit_code) *exit_code = -256;
    return -1; // Error
#endif
}
PROCEDURE(pipeexitcode) {
    rxinteger rproc = GETINT(ARG0);
    ChildProcess *proc = (ChildProcess *) rproc;
    if(proc->status=='t') RETURNINTX(-1024); // task was terminated by timeout, ther is no ecit code
    int timeout=5000;
    int exit_code = -9999;
    int rc = wait_child_process(proc, timeout /*block max 5 secs*/, &exit_code);
    if (rc == 1) RETURNINTX(exit_code);   // Child exited, return its exit code
    if (rc == 0) RETURNINTX(-4);      // Still running (should not happen with block=1)
    RETURNINTX(-8);      // Error
    ENDPROC
}
/* ----------------------------------------------------------------------------
 * CREXX PIPESTATUS
 * ----------------------------------------------------------------------------
 */
PROCEDURE(pipestatus) {
    rxinteger rproc = GETINT(ARG0);
    char rbuf[32];
    ChildProcess *proc = (ChildProcess *) rproc;
    int status = wait_child_process(proc, 0,0);  // Non-blocking check
     // Optionally, update your .running flag:
    if (status == 1 || status == -1) {
        proc->running = 0;
        proc->status  = 'i';
    }
    snprintf(rbuf, sizeof(rbuf), "%d %c %jd", proc->running, proc->status,(rxinteger) proc);
    RETURNSTRX(rbuf); // 1=still running, 0=exited or error
    ENDPROC
}

/* ----------------------------------------------------------------------------
 * CREXX PIPECANCEL
 * ----------------------------------------------------------------------------
 */
PROCEDURE(pipecancel) {
    rxinteger rproc = GETINT(ARG0);
    ChildProcess *proc = (ChildProcess *)rproc;
    int rc = cancel_child_process(proc);
    RETURNINTX(rc);
    ENDPROC
}
// Functions to be provided to rexx
LOADFUNCS
    ADDPROC(pipecreate,  "pipe.pipecreate",  "b",    ".int" ,"mode=r");
    ADDPROC(piperun,     "pipe.piperun",     "b",    ".int" ,"proc=.int,cmd=.string,mode='R'");
    ADDPROC(pipesend,    "pipe.pipesend",    "b",    ".int" ,"proc=.int,string=.string");
    ADDPROC(pipeexitcode,"pipe.pipeexitcode","b",    ".int" ,"proc=.int");
    ADDPROC(pipeget,     "pipe.pipeget",     "b",    ".int" ,"proc=.int, expose array=.string[],timeout=0");
    ADDPROC(pipestatus,  "pipe.pipestatus",  "b",    ".string","proc=.int");
    ADDPROC(pipecancel,  "pipe.pipecancel",  "b",    ".int" ,"proc=.int");
    ADDPROC(pipeclose,   "pipe.pipeclose",   "b",    ".int" ,"proc=.int");
ENDLOADFUNCS
