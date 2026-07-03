/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Structured compiler diagnostics.
 */

#ifndef CREXX_RXCP_DIAG_H
#define CREXX_RXCP_DIAG_H

#include <stddef.h>

typedef struct RxcpDiagnosticParam {
    char *name;
    char *value;
} RxcpDiagnosticParam;

typedef struct RxcpDiagnostic {
    char *code;
    RxcpDiagnosticParam *params;
    size_t param_count;
} RxcpDiagnostic;

int rxcp_diag_set_mode(const char *mode);
int rxcp_diag_set_locale(const char *locale);
const char *rxcp_diag_mode(void);
char *rxcp_diag_effective_locale(void);
RxcpDiagnostic *rxcp_diag_create(const char *code);
RxcpDiagnostic *rxcp_diag_clone(const RxcpDiagnostic *diag);
void rxcp_diag_free(RxcpDiagnostic *diag);
int rxcp_diag_add_param(RxcpDiagnostic *diag, const char *name, const char *value);
int rxcp_diag_equal(const RxcpDiagnostic *left, const RxcpDiagnostic *right);
char *rxcp_diag_render_raw(const RxcpDiagnostic *diag, const char *fallback);
char *rxcp_diag_render(const RxcpDiagnostic *diag, const char *fallback);
char *rxcp_diag_int_string(int value);
char *rxcp_diag_levelc_code(const char *standard_code);

#endif
