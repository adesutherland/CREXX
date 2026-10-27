/* cREXX License (MIT). No command-resource owner in the single-thread port. */
#include "rxcrexxcmd.h"

/* Keep this fallback in its own archive member. An embedding that supplies the
 * command-resource service resolves the symbol without pulling this member. */
int rxcrexxcmd_active_resource_snapshot(char **working_directory) {
    if (working_directory) *working_directory = NULL;
    return -1;
}
