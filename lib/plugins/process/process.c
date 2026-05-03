//
// Minimal Cross-Platform Process API (Handle-based, like sockets)
// Supports: Windows & Linux/UNIX
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <winnt.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#endif

#include <time.h>  // for time_t
#include "crexxpa.h"  // your framework header

#ifdef _WIN32
#define wait(ms) Sleep(ms);
#else
#define wait(ms) usleep(ms*1000)
#endif

#define MAX_PROCS 256

#define PROCESS_OUTPUT_MAX (64*1024)  // Max output buffer size

void print_stack_info(void) {
#ifdef _WIN32
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(&mbi, &mbi, sizeof(mbi));

    char *stack_low  = (char*)mbi.AllocationBase;
    char *stack_high = (char*)mbi.BaseAddress + mbi.RegionSize;
    SIZE_T reserved_size = stack_high - stack_low;
    SIZE_T committed_size = mbi.RegionSize;

    printf("=== Stack info (Windows) ===\n");
    printf("Reserved base : %p\n", (void*)stack_high);
    printf("Reserved limit: %p\n", (void*)stack_low);
    printf("Reserved size : %zu bytes (%.2f KB)\n",
           reserved_size, reserved_size / 1024.0);
    printf("Committed size: %zu bytes (%.2f KB)\n",
           committed_size, committed_size / 1024.0);

    if (reserved_size < 128 * 1024)
        printf("⚠️  Warning: small stack (%.1f KB) — consider smaller buffers.\n",
               reserved_size / 1024.0);

#else
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);

    void *stack_addr;
    size_t stack_size;
    pthread_attr_getstack(&attr, &stack_addr, &stack_size);
    pthread_attr_destroy(&attr);

    printf("=== Stack info (POSIX) ===\n");
    printf("Stack base   : %p\n", stack_addr);
    printf("Stack size   : %zu bytes (%.2f MB)\n",
           stack_size, stack_size / (1024.0 * 1024.0));

    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0)
        printf("ulimit -s    : %zu KB\n", rl.rlim_cur / 1024);
#endif
}

typedef struct {
#ifdef _WIN32
    HANDLE handle;
    DWORD pid;
    HANDLE hStdOutRead;
    HANDLE hStdInWrite;   /* parent write handle to child's stdin */
#else
    pid_t pid;
    int   fd_out;
    int   fd_in;          /* parent write fd for child's stdin */
#endif
    int status;
    int exited; // 1 = exited, 0 = running
    int last_error;
    char last_error_msg[512];
    char output[PROCESS_OUTPUT_MAX];
    int output_len;
    int stream_pos;
    int timeout_ms;       // Per-process timeout in ms (0 = none)
    time_t start_time;    // When the process was started (seconds)
} ExtProcess;

static ExtProcess *proc_table[MAX_PROCS] = { 0 };

/* --- handle pool helpers --- */
static int alloc_proc_handle(ExtProcess *p) {
    for (int i = 1; i < MAX_PROCS; ++i) {
        if (!proc_table[i]) {
            proc_table[i] = p;
            return i;
        }
    }
    return -8; // no space
}

static ExtProcess *get_proc(int handle) {
    if (handle <= 0 || handle >= MAX_PROCS) return NULL;
    return proc_table[handle];
}

static void free_proc_handle(int handle) {
    if (handle > 0 && handle < MAX_PROCS) {
        proc_table[handle] = NULL;
    }
}

/* --- free process resources --- */
void processfreeh(ExtProcess *p) {
    if (!p) return;
#ifdef _WIN32
    if (p->handle) CloseHandle(p->handle);
    if (p->hStdOutRead) CloseHandle(p->hStdOutRead);
    if (p->hStdInWrite) CloseHandle(p->hStdInWrite);
#else
    if (p->fd_out >= 0) close(p->fd_out);
    if (p->fd_in  >= 0) close(p->fd_in);
#endif
    free(p);
}

/* --- create process (with stdout and stdin pipes) --- */
ExtProcess* ext_process_create(const char *cmdline, int capture_output) {
    ExtProcess *p = (ExtProcess*)malloc(sizeof(ExtProcess));
    if (!p) return NULL;
    memset(p, 0, sizeof(ExtProcess));
    /* initialize fd/handle sentinel values */
#ifdef _WIN32
    p->hStdOutRead = NULL;
    p->hStdInWrite = NULL;
#else
    p->fd_out = -1;
    p->fd_in  = -1;
#endif

#ifdef _WIN32
    HANDLE hStdOutRead = NULL, hStdOutWrite = NULL;
    HANDLE hStdInRead = NULL,  hStdInWrite = NULL;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    /* Create stdout pipe (child -> parent) if requested */
    if (capture_output) {
        if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
            free(p);
            return NULL;
        }
        /* parent will read from hStdOutRead, so prevent it being inherited */
        SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
    }

    /* Create stdin pipe (parent -> child) always (to support sendinput). */
    if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
        if (hStdOutRead) CloseHandle(hStdOutRead);
        if (hStdOutWrite) CloseHandle(hStdOutWrite);
        free(p);
        return NULL;
    }
    /* parent will write to hStdInWrite; prevent it being inherited */
    SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = capture_output ? STARTF_USESTDHANDLES : STARTF_USESTDHANDLES;
    si.hStdOutput = capture_output ? hStdOutWrite : GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError  = capture_output ? hStdOutWrite : GetStdHandle(STD_ERROR_HANDLE);
    si.hStdInput  = hStdInRead; /* give child the read end */

    ZeroMemory(&pi, sizeof(pi));
    char *cmd = _strdup(cmdline);
    BOOL ok = CreateProcessA(
            NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi
    );
    if (ok) {
        p->handle = pi.hProcess;
        p->pid = pi.dwProcessId;
        p->status = 0;
        p->exited = 0;
        p->last_error = 0;
        p->last_error_msg[0] = 0;
        p->hStdOutRead = hStdOutRead;
        p->hStdInWrite = hStdInWrite;
        CloseHandle(pi.hThread);
        /* parent should close handles it doesn't need:
           child inherited hStdOutWrite and hStdInRead; parent keeps hStdOutRead and hStdInWrite */
        if (capture_output) CloseHandle(hStdOutWrite);
        CloseHandle(hStdInRead);
    } else {
        p->handle = NULL;
        p->pid = 0;
        p->status = -1;
        p->exited = 1;
        p->last_error = (int)GetLastError();
        snprintf(p->last_error_msg, sizeof(p->last_error_msg),
                 "CreateProcess failed: %d", p->last_error);
        if (hStdOutRead) CloseHandle(hStdOutRead);
        if (hStdOutWrite) CloseHandle(hStdOutWrite);
        if (hStdInRead) CloseHandle(hStdInRead);
        if (hStdInWrite) CloseHandle(hStdInWrite);
        free(p);
        p = NULL;
    }
    free(cmd);

#else
    int fd_out[2] = { -1, -1 };
    int fd_in[2]  = { -1, -1 };

    /* create stdout pipe (child -> parent) if requested */
    if (capture_output) {
        if (pipe(fd_out) != 0) {
            free(p);
            return NULL;
        }
    }

    /* create stdin pipe (parent -> child) always to support sendinput */
    if (pipe(fd_in) != 0) {
        if (fd_out[0] >= 0) { close(fd_out[0]); fd_out[0] = -1; }
        if (fd_out[1] >= 0) { close(fd_out[1]); fd_out[1] = -1; }
        free(p);
        return NULL;
    }

    pid_t pid = fork();
    if (pid == 0) {
        /* child */
        if (capture_output) {
            close(fd_out[0]);           /* close parent's read end */
            dup2(fd_out[1], STDOUT_FILENO);
            dup2(fd_out[1], STDERR_FILENO);
            close(fd_out[1]);
        }
        /* stdin: child uses fd_in[0] */
        close(fd_in[1]); /* close parent's write end */
        dup2(fd_in[0], STDIN_FILENO);
        close(fd_in[0]);

        execl("/bin/sh", "sh", "-c", cmdline, (char*)NULL);
        _exit(127);
    } else if (pid > 0) {
        p->pid = pid;
        p->status = 0;
        p->exited = 0;
        p->last_error = 0;
        p->last_error_msg[0] = 0;
        if (capture_output) {
            close(fd_out[1]); /* close child's write end in parent */
            p->fd_out = fd_out[0];
            fcntl(p->fd_out, F_SETFL, O_NONBLOCK);
        } else {
            p->fd_out = -1;
        }
        /* parent keeps fd_in[1] to write to child's stdin */
        close(fd_in[0]);
        p->fd_in = fd_in[1];
        /* Optional: make non-blocking if you want (commented out)
           int flags = fcntl(p->fd_in, F_GETFL, 0);
           fcntl(p->fd_in, F_SETFL, flags | O_NONBLOCK);
        */
    } else {
        /* fork failed */
        p->pid = -1;
        p->status = -1;
        p->exited = 1;
        p->last_error = errno;
        snprintf(p->last_error_msg, sizeof(p->last_error_msg),
                 "fork failed: %d", errno);
        if (fd_out[0] >= 0) close(fd_out[0]);
        if (fd_out[1] >= 0) close(fd_out[1]);
        if (fd_in[0] >= 0) close(fd_in[0]);
        if (fd_in[1] >= 0) close(fd_in[1]);
        free(p);
        p = NULL;
    }
#endif

    if (p) {
        p->output[0] = 0;
        p->output_len = 0;
        p->stream_pos = 0;
        p->timeout_ms = 0;
        p->start_time = time(0);
    }
    return p;
}

/* --- read available bytes (chunk mode, no CR/LF logic) --- */
int ext_process_readchunk(ExtProcess *p, char *buf, int buflen) {
    if (!p || !buf || buflen <= 1) return 0;

    /* Make space in the rolling buffer if we've already consumed some */
    if (p->stream_pos > 0 && p->stream_pos < p->output_len) {
        int remain = p->output_len - p->stream_pos;
        memmove(p->output, p->output + p->stream_pos, (size_t)remain);
        p->output_len = remain;
        p->stream_pos = 0;
    } else if (p->stream_pos >= p->output_len) {
        p->stream_pos = p->output_len = 0;
    }

    /* Try to pull more from the child (non-blocking) */
#ifdef _WIN32
    if (p->hStdOutRead) {
        DWORD read = 0;
        int room = PROCESS_OUTPUT_MAX - 1 - p->output_len;
        if (room > 0) {
            BOOL ok = ReadFile(p->hStdOutRead, p->output + p->output_len, (DWORD)room, &read, NULL);
            if (ok && read > 0) {
                p->output_len += (int)read;
                p->output[p->output_len] = 0;
            }
        }
    }
#else
    if (p->fd_out >= 0) {
        int room = PROCESS_OUTPUT_MAX - 1 - p->output_len;
        if (room > 0) {
            int n = (int)read(p->fd_out, p->output + p->output_len, (size_t)room);
            if (n > 0) {
                p->output_len += n;
                p->output[p->output_len] = 0;
            }
        }
    }
#endif

    /* Serve whatever we have right now */
    int avail = p->output_len - p->stream_pos;
    if (avail <= 0) return 0;

    int n = (avail < (buflen - 1)) ? avail : (buflen - 1);
    memcpy(buf, p->output + p->stream_pos, (size_t)n);
    buf[n] = 0;
    p->stream_pos += n;

    /* If we’ve consumed everything, reset to keep the buffer compact */
    if (p->stream_pos >= p->output_len) {
        p->stream_pos = 0;
        p->output_len = 0;
        p->output[0] = 0;
    }
    return n;
}

/* --- read next line (streaming) --- */
int ext_process_readoutput(ExtProcess *p, char *buf, int buflen) {
    if (!p) return 0;
    /* try to read more from the child non-blocking */
#ifdef _WIN32
    if (p->hStdOutRead) {
        DWORD read = 0;
        static char temp[1024];
        BOOL ok = ReadFile(p->hStdOutRead, temp, sizeof(temp)-1, &read, NULL);
        if (ok && read > 0 && (p->output_len + (int)read < PROCESS_OUTPUT_MAX)) {
            memcpy(p->output + p->output_len, temp, (size_t)read);
            p->output_len += (int)read;
            p->output[p->output_len] = 0;
        }
    }
#else
    if (p->fd_out >= 0) {
        static char temp[1024];
        int n = (int)read(p->fd_out, temp, sizeof(temp)-1);
        if (n > 0 && (p->output_len + n < PROCESS_OUTPUT_MAX)) {
            memcpy(p->output + p->output_len, temp, n);
            p->output_len += n;
            p->output[p->output_len] = 0;
        }
    }
#endif

    /* find next newline from stream_pos */
    if (p->stream_pos < p->output_len) {
        char *start = p->output + p->stream_pos;
        char *newline = strchr(start, '\n');
        if (newline) {
            int linelen = (int)((newline - start) + 1);
            if (linelen > buflen - 1) linelen = buflen - 1;
            memcpy(buf, start, linelen);
            buf[linelen] = 0;
            p->stream_pos += linelen;
            return linelen;
        }
    }
    return 0;
}

/* --- basic lifecycle helpers --- */
int ext_process_isrunning(ExtProcess *p) {
    if (!p) return 0;
#ifdef _WIN32
    DWORD code = 0;
    if (GetExitCodeProcess(p->handle, &code)) {
        return (code == STILL_ACTIVE) ? 1 : 0;
    }
    return 0;
#else
    int status = 0;
    pid_t result = waitpid(p->pid, &status, WNOHANG);
    if (result == 0) return 1;           /* still running */
    if (result == p->pid) {
        /* record status */
        p->status = status;
        p->exited = 1;
        return 0;
    }
    return 0;
#endif
}

int ext_process_wait(ExtProcess *p, int timeout_ms) {
    if (!p) return -1;
#ifdef _WIN32
    DWORD rc = WaitForSingleObject(p->handle, timeout_ms ? (DWORD)timeout_ms : INFINITE);
    return (rc == WAIT_OBJECT_0) ? 0 : -1;
#else
    int waited = 0, interval = 10;
    while (ext_process_isrunning(p) == 1) {
        if (timeout_ms && waited >= timeout_ms) return -1;
        usleep(interval * 1000);
        waited += interval;
    }
    if (ext_process_isrunning(p) == -1) return -1;
    return 0;
#endif
}

int ext_process_kill(ExtProcess *p) {
    if (!p) return -1;
#ifdef _WIN32
    return TerminateProcess(p->handle, 1) ? 0 : -1;
#else
    return kill(p->pid, SIGKILL);
#endif
}

/* --- write to child's stdin --- */
int ext_process_sendinput(ExtProcess *p, const char *data, int len) {
    if (!p || !data || len <= 0) return -1;
#ifdef _WIN32
    if (!p->hStdInWrite) return -1;
    DWORD written = 0;
    BOOL ok = WriteFile(p->hStdInWrite, data, (DWORD)len, &written, NULL);
    return ok ? (int)written : -1;
#else
    if (p->fd_in < 0) return -1;
    ssize_t w = write(p->fd_in, data, (size_t)len);
    return (w >= 0) ? (int)w : -1;
#endif
}

/* --- close parent's stdin write end to signal EOF to child --- */
int ext_process_closeinput(ExtProcess *p) {
    if (!p) return -1;
#ifdef _WIN32
    if (p->hStdInWrite) {
        CloseHandle(p->hStdInWrite);
        p->hStdInWrite = NULL;
    }
    return 0;
#else
    if (p->fd_in >= 0) {
        close(p->fd_in);
        p->fd_in = -1;
    }
    return 0;
#endif
}

int ext_process_getoutput(ExtProcess *p, char *buf, int buflen) {
    if (!p) return -1;
    if (p->output_len > 0) {
        int n = (p->output_len < buflen-1) ? p->output_len : buflen-1;
        memcpy(buf, p->output, n);
        buf[n] = 0;
        return n;
    }
#ifdef _WIN32
    if (p->hStdOutRead) {
        DWORD read = 0;
        BOOL ok = ReadFile(p->hStdOutRead, p->output, PROCESS_OUTPUT_MAX-1, &read, NULL);
        if (ok && read > 0) {
            p->output[read] = 0;
            p->output_len = (int)read;
            int n = (read < buflen-1) ? read : buflen-1;
            memcpy(buf, p->output, n);
            buf[n] = 0;
            return n;
        }
    }
#else
    if (p->fd_out >= 0) {
        int n = (int)read(p->fd_out, p->output, PROCESS_OUTPUT_MAX-1);
        if (n > 0) {
            p->output[n] = 0;
            p->output_len = n;
            int m = (n < buflen-1) ? n : buflen-1;
            memcpy(buf, p->output, m);
            buf[m] = 0;
            return m;
        }
    }
#endif
    return 0;
}

/* ------------------------------------------------------------------
   ext_process_peekoutput()
   Non-consuming read of the current stdout buffer.
   Does NOT advance stream_pos; intended for quick status inspection.
------------------------------------------------------------------ */
int ext_process_peekoutput(ExtProcess *p, char *buf, int buflen) {
    if (!p || !buf || buflen <= 1) return 0;

    // If there’s already buffered output, copy it.
    if (p->output_len > 0) {
        int n = (p->output_len < buflen - 1) ? p->output_len : buflen - 1;
        memcpy(buf, p->output, n);
        buf[n] = 0;
        return n;
    }

#ifdef _WIN32
    // Try to read more data non-blocking into temp buffer, but do NOT move stream_pos
    if (p->hStdOutRead) {
        DWORD read = 0;
        char tmp[1024];
        BOOL ok = PeekNamedPipe(p->hStdOutRead, tmp, sizeof(tmp)-1, &read, NULL, NULL);
        if (ok && read > 0) {
            int n = (read < buflen - 1) ? read : buflen - 1;
            memcpy(buf, tmp, n);
            buf[n] = 0;
            return n;
        }
    }
#else
    // On Unix, simulate peek by doing a non-blocking read and immediately re-inserting data
    if (p->fd_out >= 0) {
        char tmp[1024];
        int n = read(p->fd_out, tmp, sizeof(tmp));
        if (n > 0) {
            // Store data back into p->output (so it's not lost)
            int room = PROCESS_OUTPUT_MAX - p->output_len - 1;
            if (n > room) n = room;
            if (n > 0) {
                memmove(p->output + p->output_len, tmp, n);
                p->output_len += n;
                p->output[p->output_len] = 0;
            }
            // Return a copy for the caller
            int m = (p->output_len < buflen - 1) ? p->output_len : buflen - 1;
            memcpy(buf, p->output, m);
            buf[m] = 0;
            return m;
        }
    }
#endif
    buf[0] = 0;
    return 0;
}



char* ext_process_lasterror(ExtProcess *p) {
    if (!p) return "No process handle";
    return p->last_error_msg;
}

int ext_process_getexitcode(ExtProcess *p) {
    if (!p) return -1;
#ifdef _WIN32
    DWORD code = 0;
    if (GetExitCodeProcess(p->handle, &code)) {
        if (code == STILL_ACTIVE) return -1; /* still running */
        return (int)code;
    }
    return -2; /* error */
#else
    if (!p->exited) return -1; /* still running */
    if (WIFEXITED(p->status))
        return WEXITSTATUS(p->status);
    else if (WIFSIGNALED(p->status))
        return 128 + WTERMSIG(p->status);
    else
        return -2;
#endif
}

/* --- watchdog (same as before) --- */
void process_watchdog_tick() {
    time_t now = time(NULL);
    for (int i = 1; i < MAX_PROCS; ++i) {
        ExtProcess *p = proc_table[i];
        if (p && p->timeout_ms > 0 && ext_process_isrunning(p)) {
            int elapsed = (int)((now - p->start_time) * 1000); /* ms */
            if (elapsed > p->timeout_ms) {
                ext_process_kill(p);
                p->timeout_ms = 0; /* only once */
                snprintf(p->last_error_msg, sizeof(p->last_error_msg),
                         "Process killed by watchdog after %d ms", elapsed);
            }
        }
    }
}

#ifdef _WIN32
#include <process.h>
unsigned __stdcall process_watchdog_thread(void *dummy) {
#else
    void* process_watchdog_thread(void *dummy) {
#endif
    while (1) {
        process_watchdog_tick();
#ifdef _WIN32
        Sleep(1000);
#else
        usleep(1000000);
#endif
    }
    return 0;
}

/* ----------------- REXX Wrappers ----------------- */

PROCEDURE(processcreate) {  /* args: cmdline(.string), capture_output(.int, optional) */
    const char *cmd = GETSTRING(ARG0);
    int capture = GETINT(ARG1);
    ExtProcess *p = ext_process_create(cmd, capture);
    if (!p) RETURNINTX(-1);
    if (p->last_error) RETURNINTX(-(100+p->last_error));
    int handle = alloc_proc_handle(p);
    if (handle < 0) {
        processfreeh(p);
        RETURNINTX(-8); /* no space */
    }
    RETURNINTX(handle);
    ENDPROC
}

PROCEDURE(processisrunning) {  /* args: handle(.int) */
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(0);
    RETURNINTX(ext_process_isrunning(p));
    ENDPROC
}

PROCEDURE(processwait) { /* args: handle(.int), timeout_ms(.int) */
    int handle = GETINT(ARG0);
    int timeout = GETINT(ARG1);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(-1);
    RETURNINTX(ext_process_wait(p, timeout));
    ENDPROC
}

PROCEDURE(processkill) { /* args: handle(.int) */
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(-1);
    RETURNINTX(ext_process_kill(p));
    ENDPROC
}

PROCEDURE(processgetoutput) {  /* args: handle(.int) */
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNSTRX("");
    static char buf[4096];
    int n = ext_process_getoutput(p, buf, sizeof(buf));
    RETURNSTRX(buf);
    ENDPROC
}

PROCEDURE(processlasterror) {  /* args: handle(.int) */
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNSTRX("No process handle");
    char * error = ext_process_lasterror(p);
    if (error[0] == '\0') RETURNSTRX("n/a");
    RETURNSTRX(error);
    ENDPROC
}

PROCEDURE(processfree) { /* args: handle(.int) */
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (p) {
        if (ext_process_isrunning(p)) ext_process_kill(p);
        processfreeh(p);
        free_proc_handle(handle);
    }
    RETURNINTX(0)
    ENDPROC
}

PROCEDURE(processfreeall) { /* args: handle(.int) */
    int j = 0;
    for (int i = 1; i < MAX_PROCS; ++i) {
        if (proc_table[i]) {
            ExtProcess *p = proc_table[i];
            processfreeh(p);
            proc_table[i] = NULL;  /* important */
            j++;
        }
    }
    RETURNINTX(j)    // return number of freed processes
    ENDPROC
}

PROCEDURE(sleep) { /* args: ms(.int) */
    wait(GETINT(ARG0));
    RETURNINTX(0)
    ENDPROC
}

PROCEDURE(processgetexitcode) {  /* args: handle(.int) */
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(-99);
    RETURNINTX(ext_process_getexitcode(p));
    ENDPROC
}

PROCEDURE(processwaitkill) { /* args: handle(.int), timeout_ms(.int) */
    int handle = GETINT(ARG0);
    int timeout = GETINT(ARG1);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(-99);
    int rc = ext_process_wait(p, timeout);
    if (rc == -1 && ext_process_isrunning(p)) {
        ext_process_kill(p);
        RETURNINTX(-2); /* killed due to timeout */
    }
    RETURNINTX(rc);
    ENDPROC
}

PROCEDURE(processlist) { /* args: (none) */
    int j = 0;
    for (int i = 1; i < MAX_PROCS; ++i) {
        if (proc_table[i]) {
            j++;
            SETARRAYHI(RETURN, j);
            SETIARRAY(RETURN, j-1, (rxinteger) proc_table[i]); /* return handle index, not pointer */
        }
    }
    ENDPROC
}

PROCEDURE(processreadoutput) {  // args: handle(.int)
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNSTRX("");

    static char buf[PROCESS_OUTPUT_MAX];
    int n = ext_process_readoutput(p, buf, sizeof(buf));
    if (n <= 0) RETURNSTRX("");
    // Normalize the line we just received (in-place on buf)
    int i, j = 0;
    for (i = 0; i < n; i++) {         // use n, not strlen(buf)
        char c = buf[i];              // <-- normalize from buf, not p->output
        if (c != '\r' && c != '\n')   // drop CR and LF
            buf[j++] = c;             // compact forward
    }
    buf[j] = 0;

    RETURNSTRX(buf);
    ENDPROC
}


/* chunk read: returns up to maxbytes (optional). No newline parsing. */
PROCEDURE(processreadchunk) {  /* args: handle(.int), maxbytes(.int, optional) */
    int handle = GETINT(ARG0);
    int maxbytes = GETINT(ARG1);
    if (maxbytes <= 0 || maxbytes > PROCESS_OUTPUT_MAX) maxbytes = 4096;

    ExtProcess *p = get_proc(handle);
    if (!p) RETURNSTRX("");

    static char *buf = NULL;  /* init */
    if (buf) {
        char *tmp = (char*)realloc(buf, (size_t)maxbytes + 1);
        if (!tmp) RETURNSTRX("");
        buf = tmp;
    } else {
        buf = (char*)malloc((size_t)maxbytes + 1);
        if (!buf) RETURNSTRX("");
    }

    int n = ext_process_readchunk(p, buf, maxbytes + 1);
    if (n < 0) buf[0] = 0;
    RETURNSTRX(buf);
    ENDPROC
}

static int watchdog_running = 0;

PROCEDURE(processsettimeout) { /* args: handle(.int), timeout_ms(.int) */
    int handle = GETINT(ARG0);
    int timeout = GETINT(ARG1);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(-99);
    p->timeout_ms = timeout;
    if (!watchdog_running) {
#ifdef _WIN32
        uintptr_t th = _beginthreadex(NULL, 0, process_watchdog_thread, NULL, 0, NULL);
        if (th) {
            CloseHandle((HANDLE)th);
            watchdog_running = 1;
        }
#else
        pthread_t tid;
            if (pthread_create(&tid, NULL, process_watchdog_thread, NULL) == 0) {
                pthread_detach(tid);
                watchdog_running = 1;
            }
#endif
    }
    RETURNINTX(0);
    ENDPROC
}


PROCEDURE(processsendinput) { /* args: handle(.int), data(.string) */
    int handle = GETINT(ARG0);
    const char *data = GETSTRING(ARG1);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(-99);
    if (!data) RETURNINTX(-1);
    int len = (int)strlen(data);
    int n = ext_process_sendinput(p, data, len);
    RETURNINTX(n);
    ENDPROC
}

PROCEDURE(processcloseinput) { /* args: handle(.int) */
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNINTX(-99);
    int rc = ext_process_closeinput(p);
    RETURNINTX(rc);
    ENDPROC
}

PROCEDURE(processstackinfo) {
    print_stack_info();
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(processpeekoutput) {  // args: handle(.int)
    int handle = GETINT(ARG0);
    ExtProcess *p = get_proc(handle);
    if (!p) RETURNSTRX("");

    static char buf[PROCESS_OUTPUT_MAX];
    int n = ext_process_peekoutput(p, buf, sizeof(buf));
    if (n <= 0) RETURNSTRX("");

    // Optional: strip CR/LF for a cleaner look
    int i, j = 0;
    for (i = 0; i < n; i++) {
        char c = buf[i];
        if (c != '\r' && c != '\n')
            buf[j++] = c;
    }
    buf[j] = 0;

    RETURNSTRX(buf);
    ENDPROC
}


/* ----------------------------------------------------------------------------
   Registration Table (for ADDPROC system)
   ---------------------------------------------------------------------------- */
LOADFUNCS
    ADDPROC(processcreate,    "process.processcreate",    "b", ".int",    "cmdline=.string,capture_output=.int");
    ADDPROC(processisrunning, "process.processisrunning", "b", ".int",    "handle=.int");
    ADDPROC(processwait,      "process.processwait",      "b", ".int",    "handle=.int,timeout_ms=.int");
    ADDPROC(processkill,      "process.processkill",      "b", ".int",    "handle=.int");
    ADDPROC(processgetoutput, "process.processgetoutput", "b", ".string", "handle=.int");
    ADDPROC(processlasterror, "process.processlasterror", "b", ".string", "handle=.int");
    ADDPROC(processfree,      "process.processfree",      "b", ".int",    "handle=.int");
    ADDPROC(processfreeall,   "process.processfreeall",   "b", ".int",    " ");
    ADDPROC(processgetexitcode,"process.processgetexitcode", "b", ".int", "handle=.int");
    ADDPROC(processlist,      "process.processlist",         "b", ".int[]"," ");
    ADDPROC(processwaitkill,  "process.waitkill", "b", ".int", "handle=.int,timeout_ms=.int");
    ADDPROC(sleep,            "process.wait",             "b", ".int",    "wait=.int");
    ADDPROC(processreadoutput,"process.processreadoutput", "b", ".string", "handle=.int");
    ADDPROC(processsettimeout,"process.processsettimeout", "b", ".int", "handle=.int,timeout_ms=.int");
    ADDPROC(processsendinput, "process.processsendinput",  "b", ".int",   "handle=.int,data=.string");
    ADDPROC(processcloseinput,"process.processcloseinput", "b", ".int",   "handle=.int");
    ADDPROC(processpeekoutput, "process.processpeekoutput", "b", ".string", "handle=.int");
    ADDPROC(processstackinfo, "process.processstackinfo",  "b", ".int",   "");
ENDLOADFUNCS