/* cREXX License (MIT). Copyright (c) 2026 Adrian Sutherland.
 * Native test of CMS platform selection, not a CMS service emulator. */
#include "platform.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); return 1; } } while (0)
int main(void) {
    const unsigned char data[]={0,1,0x7f,0x80,0xff};
    FILE *f=openfile("CMSPROBE","dat",0,"wb");
    CHECK(f);
    CHECK(fwrite(data,1,sizeof data,f)==sizeof data);
    CHECK(fclose(f)==0);
    CHECK(fileexists("CMSPROBE","dat",0));
    CHECK(fileexists("CMSPROBE.dat","dat","."));
    CHECK(fileexists("CMSPROBE","dat","missing-directory;."));
    CHECK(!fileexists("CMSPROBE","missing",0));
    f=openfile("CMSPROBE","dat",".","rb");
    CHECK(f);
    size_t n=0;
    char *bytes=file2buf(f,&n);
    CHECK(bytes && n==sizeof data && !memcmp(bytes,data,n));
    CHECK(bytes[n]==0 && bytes[n+1]==0);
    free(bytes);
    CHECK(fclose(f)==0);
    CHECK(remove("CMSPROBE.dat")==0);
    char *p=exefqname(); CHECK(p && !*p); free(p);
    p=exepath(); CHECK(p && !*p); free(p);
    void *dir=(void *)&n;
    errno=0; CHECK(!dirfstfl(".",0,"dat",&dir) && !dir && errno==ENOSYS);
    errno=0; CHECK(!dirnxtfl(&dir) && !dir && errno==ENOSYS);
    dirclose(&dir); CHECK(!dir);
    platform_term_save(); platform_term_restore(); platform_install_signal_handlers();
    /* Empty input still has the two sentinels. Seekable input is rewound. */
    f=tmpfile(); CHECK(f);
    bytes=file2buf(f,&n); CHECK(bytes && !n && !bytes[0] && !bytes[1]);
    free(bytes);
    CHECK(fwrite(data,1,sizeof data,f)==sizeof data);
    bytes=file2buf(f,&n); CHECK(bytes && n==sizeof data && !memcmp(bytes,data,n));
    free(bytes); CHECK(fclose(f)==0);
    /* A pipe exercises the non-seeking path and multiple growth steps. */
    {
        int pipes[2]; unsigned char input[3073]; size_t i;
        for (i=0;i<sizeof input;i++) input[i]=(unsigned char)i;
        CHECK(pipe(pipes)==0);
        CHECK(write(pipes[1],input,sizeof input)==sizeof input);
        CHECK(close(pipes[1])==0);
        f=fdopen(pipes[0],"rb"); CHECK(f);
        bytes=file2buf(f,&n);
        CHECK(bytes && n==sizeof input && !memcmp(bytes,input,n));
        CHECK(!bytes[n] && !bytes[n+1]);
        free(bytes); CHECK(fclose(f)==0);
    }
    /* A read error must not be mistaken for an empty input file. */
    f=fopen("CMSERROR.dat","wb"); CHECK(f);
    CHECK(fwrite(data,1,sizeof data,f)==sizeof data && fflush(f)==0);
    n=999; bytes=file2buf(f,&n); CHECK(!bytes && !n && ferror(f));
    CHECK(fclose(f)==0 && remove("CMSERROR.dat")==0);
    puts("PASS: CMS platform selection and standard I/O lookup");
    return 0;
}
