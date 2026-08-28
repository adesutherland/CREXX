/* POSIX child setup failures must reach the caller with their stage and errno. */

#include "rxvmintp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    static const char *const argv[] = {
        "/bin/echo", "this-command-must-not-run"
    };
    int command_rc = -1;
    int spawn_rc;
    char *error_text = NULL;

    spawn_rc = shellspawn_argv_snapshot(
            argv, 2, NULL, NULL, NULL,
            "/crexx-rxspawn-diagnostic-directory-does-not-exist",
            NULL, 0, NULL, NULL, NULL, NULL,
            &command_rc, &error_text);

    if (spawn_rc != SHELLSPAWN_FAILURE || !error_text ||
            !strstr(error_text,
                    "Failure entering child working directory") ||
            !strstr(error_text, "RC=2")) {
        fprintf(stderr,
                "FAIL: child setup diagnostic was not specific: "
                "spawn=%d command=%d error=%s\n",
                spawn_rc, command_rc, error_text ? error_text : "(none)");
        free(error_text);
        return 1;
    }

    printf("PASS: POSIX child setup diagnostic names stage and errno\n");
    free(error_text);
    return 0;
}
