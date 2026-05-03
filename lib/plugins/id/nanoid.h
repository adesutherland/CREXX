#ifndef NANOID_H
#define NANOID_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Default NanoID length (per original library): 21 URL-safe chars */
#ifndef NANOID_DEFAULT_SIZE
#define NANOID_DEFAULT_SIZE 21
#endif

/* Generates a NanoID using the URL-safe alphabet [A-Za-z0-9_-].
   Writes exactly NANOID_DEFAULT_SIZE chars + NUL.
   Returns 1 on success, 0 on failure. */
int nanoid_generate(char out[NANOID_DEFAULT_SIZE + 1]);

/* Generic variant: custom length and/or custom alphabet.
   - out: caller-provided buffer of size (len + 1)
   - len: number of characters to generate (1..512 recommended)
   - alphabet: pointer to allowed characters (2..255 unique chars)
   - alphabet_len: number of chars in alphabet
   Returns 1 on success, 0 on failure. */
int nanoid_generate_custom(char *out, size_t len,
                           const char *alphabet, size_t alphabet_len);

#ifdef __cplusplus
}
#endif

#endif /* NANOID_H */
