/* Current raw native-service handoff, version 1. No character conversion or
 * cREXX naming policy belongs in these services. See native-raw-services.md.
 * Both backends implement the same declarations with their own prefix. */
#ifndef CREXX_NATIVE_RAW_H
#define CREXX_NATIVE_RAW_H
#include <stddef.h>

#define LAB_RAW_API_VERSION 1
#define LAB_RAW_READ 1u
#define LAB_RAW_WRITE 2u
#define LAB_RAW_APPEND 4u
#define LAB_RAW_RECORDS 8u
/* flags READ or WRITE, optionally APPEND with WRITE; RECORDS selects explicit
 * logical records. Without RECORDS, concatenate/preserve native record payloads
 * as bytes, without inserted or removed delimiters. Unsupported modes ENOTSUP. */
typedef void lab_raw_file;
typedef void lab_raw_directory;

#define LAB_DECLARE_RAW(prefix) \
lab_raw_file *prefix##_open(const unsigned char *name, size_t length, unsigned flags, size_t *record_capacity); \
lab_raw_file *prefix##_standard(int descriptor, unsigned flags, size_t *record_capacity); \
int prefix##_read(lab_raw_file *, unsigned char *, size_t capacity, size_t *count, int *record_end, int *eof); \
ptrdiff_t prefix##_write(lab_raw_file *, const unsigned char *, size_t count, int record_end); \
int prefix##_flush(lab_raw_file *); \
int prefix##_close(lab_raw_file *); \
lab_raw_directory *prefix##_directory_open(const unsigned char *name, size_t length); \
int prefix##_directory_next(lab_raw_directory *, unsigned char *, size_t capacity, size_t *length); \
int prefix##_directory_close(lab_raw_directory *); \
int prefix##_argument_count(void); \
int prefix##_argument(size_t index, unsigned char *, size_t capacity, size_t *length); \
int prefix##_environment(const unsigned char *name, size_t name_length, unsigned char *, size_t capacity, size_t *length); \
ptrdiff_t prefix##_stderr(const unsigned char *, size_t count, int record_end)

LAB_DECLARE_RAW(lab_cms_raw);
LAB_DECLARE_RAW(lab_tso_raw);
#undef LAB_DECLARE_RAW
#endif
