/* Issue #701. Include the production implementation to stop between redirect
 * creation and launch, without adding test hooks or public runtime API. */
#ifndef RXSPAWN_TEST_SOURCE
#define RXSPAWN_TEST_SOURCE "../rxspawn.c"
#endif
#ifndef _WIN32
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
static int test_pipe(int handles[2]);
static int test_fcntl(int fd, int cmd, ...);
#define pipe test_pipe
#define fcntl test_fcntl
#endif
#include RXSPAWN_TEST_SOURCE
#ifndef _WIN32
#undef pipe
#undef fcntl
#endif
#include <stdint.h>

static int failures;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); failures++; } } while (0)

#ifndef _WIN32
#include <poll.h>

static int observe_creation, creation_unlocked, fail_flag, created[2];
static int test_pipe(int handles[2]) {
    int rc = pipe(handles);
    if (rc == 0 && observe_creation) {
        int lock_rc = pthread_mutex_trylock(&rxspawn_posix_launch_mutex);
        created[0] = handles[0]; created[1] = handles[1];
        if (!lock_rc) {
            creation_unlocked++;
            pthread_mutex_unlock(&rxspawn_posix_launch_mutex);
        } else if (lock_rc != EBUSY) abort();
    }
    return rc;
}
static int test_fcntl(int fd, int cmd, ...) {
    int arg;
    va_list args;
    if (cmd == F_GETFD || cmd == F_GETFL) return fcntl(fd, cmd);
    va_start(args, cmd); arg = va_arg(args, int); va_end(args);
    if (cmd == F_SETFD && fail_flag && --fail_flag == 0) {
        errno = EIO; return -1;
    }
    return fcntl(fd, cmd, arg);
}
#ifndef RXSPAWN_TEST_BASELINE
static void creation_controls(void) {
    int handles[2], rc, i, saved[3];
    observe_creation = 1;
    fail_flag = 2; /* second end's flag setup fails */
    rc = rxspawn_private_pipe(handles);
    CHECK(rc == -1 && errno == EIO, "pipe flag failure keeps original errno");
    CHECK(fcntl(created[0], F_GETFD) == -1 && errno == EBADF,
          "flag failure closes read end");
    CHECK(fcntl(created[1], F_GETFD) == -1 && errno == EBADF,
          "flag failure closes write end");
    CHECK(creation_unlocked == 0, "launch mutex covers creation-to-flag window");
    rc = pthread_mutex_trylock(&rxspawn_posix_launch_mutex);
    CHECK(rc == 0, "failure releases launch mutex");
    if (!rc) pthread_mutex_unlock(&rxspawn_posix_launch_mutex);
    observe_creation = 0;
    for (i = 0; i < 3; i++) saved[i] = dup(i);
    for (i = 0; i < 3; i++) close(i);
    rc = rxspawn_private_pipe(handles);
    for (i = 0; i < 3; i++) { dup2(saved[i], i); close(saved[i]); }
    CHECK(rc == 0 && handles[0] > 2 && handles[1] > 2,
          "closed standard streams cannot alias internal pipes");
    if (rc == 0) { close(handles[0]); close(handles[1]); }
}
#endif


static void private_pipe(int p[2]) {
    if (pipe(p) || fcntl(p[0], F_SETFD, FD_CLOEXEC) ||
            fcntl(p[1], F_SETFD, FD_CLOEXEC)) abort();
}
static void init_redirect(REDIRECT *r) {
    memset(r, 0, sizeof(*r));
    r->hRead = r->hWrite = -1;
}
static void init_child(SHELLDATA *d, const char *path, char **argv) {
    memset(d, 0, sizeof(*d));
    d->file_path = (char *)path;
    d->argv = argv;
}
struct wait_state { SHELLDATA *data; int done; };
static void *wait_owner(void *arg) {
    struct wait_state *s = arg;
    WaitForProcess(s->data);
    if (write(s->done, "d", 1) != 1) abort();
    return NULL;
}
static int readable(int fd) {
    struct pollfd p = {fd, POLLIN, 0};
    int rc;
    do { rc = poll(&p, 1, 30000); } while (rc < 0 && errno == EINTR);
    return rc == 1;
}

int main(int argc, char **argv) {
    REDIRECT owner_out, sibling_in, sibling_out;
    REDIRECT_COMPLETION *capture;
    value receiver = {0};
    SHELLDATA owner, sibling;
    int input[2], output[2], done[2], status;
    char fd_text[32], byte;
    char *error = NULL;
    char *owner_argv[] = {argv[0], "owner", NULL};
    char *sibling_argv[] = {argv[0], "sibling", fd_text, NULL};
    pthread_t waiter;
    struct wait_state state;
    if (argc > 1 && !strcmp(argv[1], "owner")) {
        return write(1, "owner output\n", 13) == 13 ? 7 : 8;
    }
    if (argc > 2 && !strcmp(argv[1], "sibling")) {
        int fd = atoi(argv[2]);
        char inherited = fcntl(fd, F_GETFD) == -1 && errno == EBADF ? 'n' : 'y';
        if (write(1, &inherited, 1) != 1) return 9;
        return read(0, &byte, 1) == 1 ? 0 : 10;
    }
#ifndef RXSPAWN_TEST_BASELINE
    creation_controls();
#endif
    init_redirect(&owner_out);
    owner_out.receiver = &receiver;
    capture = redirect_completion_create(REDIRECT_TRANSFER_OUTPUT_STRING);
    if (!capture || redirect_pipe_start(&owner_out, capture, 1)) return 11;
    /* B is forced to fork while A's child endpoint is still owned by parent. */
    snprintf(fd_text, sizeof(fd_text), "%d", owner_out.hWrite);
    CHECK((fcntl(owner_out.hWrite, F_GETFD) & FD_CLOEXEC) != 0,
          "child pipe end is close-on-exec before launch");
    CHECK((fcntl(capture->io_handle, F_GETFD) & FD_CLOEXEC) != 0,
          "parent pipe end is close-on-exec before launch");
    private_pipe(input); private_pipe(output); private_pipe(done);
    init_redirect(&sibling_in); init_redirect(&sibling_out);
    sibling_in.hRead = input[0]; sibling_out.hWrite = output[1];
    init_child(&sibling, argv[0], sibling_argv);
    sibling.pInput = &sibling_in; sibling.pOutput = &sibling_out;
    if (launchChild(&sibling, &error)) { fprintf(stderr, "%s\n", error); return 12; }
    close(input[0]); sibling_in.hRead = -1;
    close(output[1]); sibling_out.hWrite = -1;
    if (!readable(output[0]) || read(output[0], &byte, 1) != 1) return 13;
    CHECK(byte == 'n', "sibling did not inherit owner's pipe writer");
    init_child(&owner, argv[0], owner_argv); owner.pOutput = &owner_out;
    if (launchChild(&owner, &error)) return 14;
    state.data = &owner; state.done = done[1];
    if (pthread_create(&waiter, NULL, wait_owner, &state)) return 15;
    /* On the defective baseline the positive descriptor observation already
     * proves why EOF is blocked. Release B to clean up without a timeout. */
    if (byte == 'n') {
        CHECK(readable(done[0]), "A completion observed while B remains alive");
        CHECK(waitpid(sibling.ChildProcessPID, &status, WNOHANG) == 0,
              "B still waits at its explicit release barrier");
    }
    CHECK(write(input[1], "x", 1) == 1, "release sibling");
    close(input[1]); close(output[0]);
    pthread_join(waiter, NULL);
    WaitForProcess(&sibling);
    CHECK(owner.waitThreadRC == 0 && owner.ChildProcessRC == 7,
          "owner exit status preserved");
    CHECK(receiver.string_length == 13 && !memcmp(receiver.string_value, "owner output\n", 13),
          "owner output fully drained");
    CHECK(sibling.ChildProcessRC == 0, "sibling handshake completed");
    clear_value(&receiver);
    close(done[0]); close(done[1]); free(error);
    printf("POSIX inheritance: %d failures\n", failures);
    return failures ? 1 : 0;
}
#else

#include <io.h>
static void windows_case(const char *self, int mask, int std_inheritable) {
    HANDLE streams[3], saved[3], ready, release, unrelated;
    DWORD ids[3] = {STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
    SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
    char root[MAX_PATH], paths[3][MAX_PATH], file[MAX_PATH], moved[MAX_PATH];
    char report[MAX_PATH], ready_name[100], release_name[100], command[4096];
    SHELLDATA child;
    REDIRECT redirects[3];
    char *error = NULL;
    DWORD n, result, before_flags[3], after_flags;
    BY_HANDLE_FILE_INFORMATION identity;
    FILE *fp;
    int inherited = -1, input_ok = 0, output_ok = 0;
    int i;
    GetTempPathA(sizeof(root), root);
    snprintf(ready_name, sizeof(ready_name), "Local\\crexx701-ready-%lu-%d-%d",
             GetCurrentProcessId(), mask, std_inheritable);
    snprintf(release_name, sizeof(release_name), "Local\\crexx701-release-%lu-%d-%d",
             GetCurrentProcessId(), mask, std_inheritable);
    ready = CreateEventA(NULL, TRUE, FALSE, ready_name);
    release = CreateEventA(NULL, TRUE, FALSE, release_name);
    if (!ready || !release) abort();
    GetTempFileNameA(root, "701", 0, file);
    GetTempFileNameA(root, "701", 0, report);
    snprintf(moved, sizeof(moved), "%s.moved", file);
    /* Freeze the FOPEN open-to-clear window: a real inheritable file which
     * denies FILE_SHARE_DELETE, as the CRT does. No timing injection needed. */
    unrelated = CreateFileA(file, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, NULL);
    if (unrelated == INVALID_HANDLE_VALUE) abort();
    GetFileInformationByHandle(unrelated, &identity);
    memset(&child, 0, sizeof(child));
    for (i = 0; i < 3; i++) {
        GetTempFileNameA(root, "701", 0, paths[i]);
        sa.bInheritHandle = std_inheritable;
        streams[i] = CreateFileA(paths[i], GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, NULL);
        if (streams[i] == INVALID_HANDLE_VALUE) abort();
        GetHandleInformation(streams[i], &before_flags[i]);
        saved[i] = GetStdHandle(ids[i]);
        SetStdHandle(ids[i], streams[i]);
        memset(&redirects[i], 0, sizeof(redirects[i]));
        redirects[i].hRead = redirects[i].hWrite = INVALID_HANDLE_VALUE;
        if (i == 0) redirects[i].hRead = streams[i];
        else redirects[i].hWrite = streams[i];
    }
    WriteFile(streams[0], "I", 1, &n, NULL);
    SetFilePointer(streams[0], 0, NULL, FILE_BEGIN);
    child.pInput = mask & 1 ? &redirects[0] : NULL;
    child.pOutput = mask & 2 ? &redirects[1] : NULL;
    child.pError = mask & 4 ? &redirects[2] : NULL;
    snprintf(command, sizeof(command), "\"%s\" probe %llu \"%s\" \"%s\" \"%s\" %lu %lu %lu",
             self, (unsigned long long)(uintptr_t)unrelated,
             ready_name, release_name, report, identity.dwVolumeSerialNumber,
             identity.nFileIndexHigh, identity.nFileIndexLow);
    child.file_path = command;
    result = launchChild(&child, &error);
    for (i = 0; i < 3; i++) SetStdHandle(ids[i], saved[i]);
    if (result != 0) {
        fprintf(stderr, "FAIL: launch mask=%d inherit=%d rc=%lu %s\n",
                mask, std_inheritable, result, error ? error : "");
        exit(20);
    }
    CHECK(WaitForSingleObject(ready, 30000) == WAIT_OBJECT_0, "child ready handshake");
    fp = fopen(report, "r");
    if (fp) { fscanf(fp, "%d %d %d", &inherited, &input_ok, &output_ok); fclose(fp); }
    CHECK(inherited == 0, "unrelated file handle excluded");
    CHECK(input_ok == 1 && output_ok == 1, "all intended standard streams work");
    CloseHandle(unrelated);
    result = MoveFileA(file, moved);
    printf("mask=%d std_inheritable=%d inherited=%d rename=%lu error=%lu\n",
           mask, std_inheritable, inherited, result, result ? 0 : GetLastError());
    CHECK(result != 0, "rename succeeds while sibling still alive");
    CHECK(WaitForSingleObject(child.ChildProcessInfo.hProcess, 0) == WAIT_TIMEOUT,
          "child remains behind release barrier during rename");
    for (i = 0; i < 3; i++) {
        CHECK(GetHandleInformation(streams[i], &after_flags) &&
              after_flags == before_flags[i], "parent stream inheritance flags unchanged");
    }
    SetEvent(release);
    CHECK(WaitForSingleObject(child.ChildProcessInfo.hProcess, 30000) == WAIT_OBJECT_0,
          "child exits after release");
    GetExitCodeProcess(child.ChildProcessInfo.hProcess, &result);
    CHECK(result == 7, "child exit code preserved");
    CloseHandle(child.ChildProcessInfo.hProcess); CloseHandle(child.ChildProcessInfo.hThread);
    for (i = 0; i < 3; i++) { CloseHandle(streams[i]); DeleteFileA(paths[i]); }
    DeleteFileA(file); DeleteFileA(moved); DeleteFileA(report);
    CloseHandle(ready); CloseHandle(release); free(error);
}
int main(int argc, char **argv) {
    int mask;
    if (argc == 9 && !strcmp(argv[1], "probe")) {
        HANDLE h = (HANDLE)(uintptr_t)strtoull(argv[2], NULL, 10);
        HANDLE ready = OpenEventA(EVENT_MODIFY_STATE, FALSE, argv[3]);
        HANDLE release = OpenEventA(SYNCHRONIZE, FALSE, argv[4]);
        DWORD flags, n;
        char byte;
        BY_HANDLE_FILE_INFORMATION info;
        int inherited = GetFileInformationByHandle(h, &info) &&
            info.dwVolumeSerialNumber == strtoul(argv[6], NULL, 10) &&
            info.nFileIndexHigh == strtoul(argv[7], NULL, 10) &&
            info.nFileIndexLow == strtoul(argv[8], NULL, 10);
        int input_ok = ReadFile(GetStdHandle(STD_INPUT_HANDLE), &byte, 1, &n, NULL)
            && n == 1 && byte == 'I';
        int output_ok = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "O", 1, &n, NULL)
            && n == 1 && WriteFile(GetStdHandle(STD_ERROR_HANDLE), "E", 1, &n, NULL) && n == 1;
        FILE *fp = fopen(argv[5], "w");
        if (!fp || !ready || !release) return 21;
        fprintf(fp, "%d %d %d", inherited, input_ok, output_ok); fclose(fp);
        SetEvent(ready);
        if (WaitForSingleObject(release, 30000) != WAIT_OBJECT_0) return 22;
        CloseHandle(ready); CloseHandle(release);
        return 7;
    }
    /* Every combination includes absent, project-worker partial (6), complete. */
    for (mask = 0; mask < 8; mask++) windows_case(argv[0], mask, 1);
#ifndef RXSPAWN_TEST_BASELINE
    for (mask = 0; mask < 8; mask++) windows_case(argv[0], mask, 0);
#endif
    printf("Windows inheritance: %d failures\n", failures);
    return failures ? 1 : 0;
}

#endif
