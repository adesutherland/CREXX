/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXCP_CONSTANT_H
#define CREXX_RXCP_CONSTANT_H

#include <stddef.h>

/* Compiler string constants retain RXAS escape spelling in the AST.  Semantic
 * evaluators must decode that spelling before using VM string routines and
 * encode the result before returning it to the ordinary constant emitter. */
unsigned char *rxcp_constant_string_decode(const char *string,
                                           size_t length,
                                           size_t *byte_length);
char *rxcp_constant_string_encode(const unsigned char *bytes,
                                  size_t byte_length,
                                  size_t *string_length);

#endif /* CREXX_RXCP_CONSTANT_H */
