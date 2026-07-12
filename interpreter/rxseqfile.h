/* Shared identity helpers for RXSEQ extraction and analysis. */

#ifndef CREXX_RXSEQFILE_H
#define CREXX_RXSEQFILE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "rxbin.h"

#define RXSEQ_FORMAT_VERSION 1
#define RXSEQ_HEADER_SIZE 48
#define RXSEQ_FLAG_OVERFLOW UINT32_C(1)

extern const unsigned char rxseq_file_magic[8];

uint64_t rxseq_hash_module_file(const module_file *file);
int rxseq_write_bytes(FILE *file, const void *data, size_t size);
int rxseq_write_u32(FILE *file, uint32_t value);
int rxseq_write_u64(FILE *file, uint64_t value);
int rxseq_write_varuint(FILE *file, uint64_t value);
int rxseq_read_bytes(FILE *file, void *data, size_t size);
int rxseq_read_u32(FILE *file, uint32_t *value);
int rxseq_read_u64(FILE *file, uint64_t *value);
int rxseq_read_varuint(FILE *file, uint64_t *value);

#endif
