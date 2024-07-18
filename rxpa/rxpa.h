//
// CREXX Plugin Architecture Library Header
//

#ifndef CREXX_RXPA_H
#define CREXX_RXPA_H

#include "crexxpa.h"

// Function to load a plugin dynamically
// - ctx is the context structure containing pointers to plugins helper functions
// - file_name is the full file name of the plugin
// Returns 0 on success
//               -1 Failed to load plugin
//               -2 Failed to call _initfuncs
int load_plugin(rxpa_initctxptr ctx, char* dir, char* file_name);

#endif //CREXX_RXPA_H
