//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <stdint.h>

#define LEFTROTATE(x, c) (((x) << (c)) | ((x) >> (32 - (c))))

// Constants are the integer part of the sines of integers (in radians) * 2^32.
const uint32_t k[] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x2441453,  0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x4881d05,  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// Per-round shift amounts
const uint32_t r[] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

PROCEDURE(md5) {
    uint32_t hash0, hash1, hash2, hash3;
    uint32_t i;
    uint8_t * tmsg = NULL;
    uint8_t digest[16];
    char * imsg=GETSTRING(ARG0);
    char result[32];
    size_t initial_len=strlen(imsg);

    // Initial hash values
    hash0 = 0x67452301;
    hash1 = 0xefcdab89;
    hash2 = 0x98badcfe;
    hash3 = 0x10325476;

    // Pre-processing
    size_t new_len, offset;
    for (new_len = initial_len + 1; new_len % 64 != 56; new_len++);
    tmsg = (uint8_t *)malloc(new_len + 8);
    memcpy(tmsg, imsg, initial_len);
    tmsg[initial_len] = 0x80; // Append the '1' bit
    memset(tmsg + initial_len + 1, 0, new_len - initial_len - 1);
    uint64_t bits_len = 8 * initial_len;
    memcpy(tmsg + new_len, &bits_len, 8);

    // Process the message in 512-bit chunks
    for (offset = 0; offset < new_len; offset += 64) {
        uint32_t *w = (uint32_t *)(tmsg + offset);
        uint32_t h0 = hash0, h1 = hash1, h2 = hash2, h3 = hash3, hwk, hindx, temp;
        // Main loop
        for (i = 0; i < 64; i++) {
            if (i < 16) {
                hwk = (h1 & h2) | (~h1 & h3);
                hindx = i;
            } else if (i < 32) {
                hwk = (h3 & h1) | (~h3 & h2);
                hindx = (5 * i + 1) % 16;
            } else if (i < 48) {
                hwk = h1 ^ h2 ^ h3;
                hindx = (3 * i + 5) % 16;
            } else {
                hwk = h2 ^ (h1 | ~h3);
                hindx = (7 * i) % 16;
            }
            temp = h3;
            h3 = h2;
            h2 = h1;
            h1 = h1 + LEFTROTATE((h0 + hwk + k[i] + w[hindx]), r[i]);
            h0 = temp;
        }
        // Add the hash of this chunk to the current result
        hash0 += h0;
        hash1 += h1;
        hash2 += h2;
        hash3 += h3;
    }

    // Produce the final digest
    memcpy(digest, &hash0, 4);
    memcpy(digest + 4, &hash1, 4);
    memcpy(digest + 8, &hash2, 4);
    memcpy(digest + 12, &hash3, 4);
    // Free dynamically allocated memory
    free(tmsg);
    for (i = 0; i < 16; i++) {
        sprintf(result+i*2,"%02x", digest[i]);
    }
    RETURNSTR(result);
    PROCRETURN
    ENDPROC
}


// File IO function definitions
LOADFUNCS
    ADDPROC(md5,"cipher.md5",   "b",  ".string", "arg0=.string");
ENDLOADFUNCS
