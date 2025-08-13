#ifndef UUIDV7_H
#define UUIDV7_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Generate a UUIDv7 into out[16]. Returns 1 on success, 0 on failure. */
int uuidv7_generate(uint8_t out[16]);

/* Format UUID bytes as canonical 8-4-4-4-12 string (out must be char[37]). */
void uuidv_to_string(const uint8_t u[16], char out[37]);

#ifdef __cplusplus
}
#endif

#endif /* UUIDV7_H */
