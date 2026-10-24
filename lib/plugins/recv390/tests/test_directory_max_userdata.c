/* Exercise the actual provider's largest legal non-ISPF directory entry. */
#include <stdio.h>
#include <string.h>

extern unsigned char *block;
extern unsigned char *dir;
extern int pos;
extern int dirpos;
extern char lastmem[8];
extern char optdir;
extern char optdirhex;
extern char optdirarray;
int printdirmem(void);

int main(void) {
    unsigned char entry[74] = {
        0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
        0, 0, 1, 0x1f
    };
    unsigned char directory[16] = {0};
    int result;
    int i;

    for (i = 0; i < 62; i++) entry[12 + i] = (unsigned char)i;
    memset(lastmem, 0xff, sizeof(lastmem));
    block = entry;
    dir = directory;
    pos = 0;
    dirpos = 0;
    optdir = '+';
    optdirhex = '-';
    optdirarray = '-';

    result = printdirmem();
    block = NULL;
    dir = NULL;
    if (result != 74 || dirpos != 16 || memcmp(directory, entry, 12) != 0) {
        fprintf(stderr, "Directory state differs: return=%d dirpos=%d\n", result, dirpos);
        return 1;
    }
    return 0;
}
