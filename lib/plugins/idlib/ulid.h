#ifndef ULID_H
#define ULID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Generate a ULID (128-bit: 48-bit ms timestamp + 80-bit randomness).
   Returns 1 on success, 0 on failure. The 16-byte binary ULID is stored in out[16]. */
int ulid_generate(uint8_t out[16]);

/* Convert a 16-byte ULID to Crockford Base32 string (26 chars + NUL). out must be char[27]. */
void ulid_to_string(const uint8_t ulid[16], char out[27]);

#ifdef __cplusplus
}
#endif

#endif /* ULID_H */
