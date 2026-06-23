/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_LEVELC_LOWER_H
#define CREXX_RXCP_LEVELC_LOWER_H

#include "rxcp_ctx.h"

const char *rxcp_levelc_compile_unsupported_message(void);
int rxcp_levelc_lower_to_canonical(Context *context, const char **reason_out);

#endif
