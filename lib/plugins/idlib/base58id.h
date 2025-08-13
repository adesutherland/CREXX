#ifndef BASE58ID_H
#define BASE58ID_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Default: 16 random bytes -> ~22–24 Base58 chars */
#ifndef BASE58ID_DEFAULT_BYTES
#define BASE58ID_DEFAULT_BYTES 16
#endif

/* Generate a Base58 ID from cryptographically random bytes.
   - out: buffer for encoded string (recommend at least 64 bytes)
   Returns 1 on success, 0 on failure. */
int base58id_generate(char *out, size_t out_cap);

/* Generate a Base58 ID from N random bytes (1..64).
   out must be large enough (64 bytes is safe for <=64 raw bytes).
   Returns 1 on success, 0 on failure. */
int base58id_generate_n(char *out, size_t out_cap, size_t nbytes);

#ifdef __cplusplus
}
#endif

#endif /* BASE58ID_H */
