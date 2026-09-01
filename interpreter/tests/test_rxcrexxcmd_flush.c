/* Verify that unredirected CREXX ADDRESS output is visible before process exit. */

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#define rx_close _close
#define rx_dup _dup
#define rx_dup2 _dup2
#define rx_fileno _fileno
#else
#include <unistd.h>
#define rx_close close
#define rx_dup dup
#define rx_dup2 dup2
#define rx_fileno fileno
#endif

#include "rxcrexxcmd.h"
#include "rxvmintp.h"

int main(void) {
    FILE *capture;
    int stdout_fd;
    int saved_stdout;
    int spawn_command_rc = -1;
    int direct_command_rc = -1;
    int spawn_rc;
    int direct_rc;
    char *spawn_error_text = NULL;
    char *direct_error_text = NULL;
    long visible_bytes;
    long direct_visible_bytes;

    capture = tmpfile();
    if (!capture) {
        fprintf(stderr, "Could not create stdout capture file\n");
        return 1;
    }
    if (setvbuf(stdout, NULL, _IOFBF, BUFSIZ) != 0) {
        fprintf(stderr, "Could not configure stdout buffering for flush test\n");
        fclose(capture);
        return 1;
    }

    stdout_fd = rx_fileno(stdout);
    saved_stdout = rx_dup(stdout_fd);
    if (saved_stdout < 0 || rx_dup2(rx_fileno(capture), stdout_fd) < 0) {
        fprintf(stderr, "Could not redirect stdout for flush test\n");
        if (saved_stdout >= 0) rx_close(saved_stdout);
        fclose(capture);
        return 1;
    }

    spawn_rc = shellspawn("echo CREXX_ADDRESS_FLUSH_READY",
                          NULL, NULL, NULL, NULL, NULL,
                          SHELLSPAWN_MODE_CREXX,
                          &spawn_command_rc, &spawn_error_text);

    if (fseek(capture, 0, SEEK_END) != 0) visible_bytes = -1;
    else visible_bytes = ftell(capture);

    direct_rc = rxcrexxcmd_execute("echo CREXX_COMMAND_FLUSH_READY",
                                   NULL, &direct_command_rc, &direct_error_text);
    if (fseek(capture, 0, SEEK_END) != 0) direct_visible_bytes = -1;
    else direct_visible_bytes = ftell(capture);

    fflush(stdout);
    rx_dup2(saved_stdout, stdout_fd);
    rx_close(saved_stdout);
    fclose(capture);

    if (spawn_rc != SHELLSPAWN_OK || spawn_command_rc != 0 ||
            direct_rc != 0 || direct_command_rc != 0 ||
            visible_bytes <= 0 || direct_visible_bytes <= visible_bytes) {
        fprintf(stderr,
                "Unredirected CREXX output was not flushed: spawn=%d/%d direct=%d/%d visible=%ld/%ld error=%s\n",
                spawn_rc, spawn_command_rc, direct_rc, direct_command_rc,
                visible_bytes, direct_visible_bytes,
                spawn_error_text ? spawn_error_text :
                (direct_error_text ? direct_error_text : ""));
        free(spawn_error_text);
        free(direct_error_text);
        return 1;
    }

    free(spawn_error_text);
    free(direct_error_text);
    return 0;
}
