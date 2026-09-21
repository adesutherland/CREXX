/* cREXX License (MIT), Copyright (c) 2026 the cREXX contributors. */
#include "platform.h"
#include <errno.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(void) {
    const char *names[] = {"alpha.crexx","alphabet.crexx","beta.crexx","alpha.rxbin"};
    void *a=NULL, *b=NULL;
    char *name;
    unsigned i, count=0;
    for (i=0;i<4;++i) { FILE *f=fopen(names[i],"wb"); CHECK(f); CHECK(fclose(f)==0); }
    name=dirfstfl(".","alpha","crexx",&a);
    CHECK(name && !strncmp(name,"alpha",5));
    name=dirfstfl(".","beta","crexx",&b);
    CHECK(name && !strcmp(name,"beta.crexx"));
    CHECK(!dirnxtfl(&b)); dirclose(&b); CHECK(!b); dirclose(&b);
    count=1;
    while ((name=dirnxtfl(&a))) { CHECK(!strncmp(name,"alpha",5)); ++count; }
    CHECK(count==2); dirclose(&a); CHECK(!a);
    CHECK(!dirfstfl(".","absent","crexx",&a)); dirclose(&a); CHECK(!a);
    errno=0;
    CHECK(!dirfstfl("missing-directory",NULL,"crexx",&a) && !a && errno==ENOENT);
    for (i=0;i<4;++i) CHECK(remove(names[i])==0);
    puts("PASS: optional CMS directory filters and independent lifetimes");
    return 0;
}
