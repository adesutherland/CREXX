#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rxcp_emit.h"

static int failures = 0;

static void expect_present(const char *output, const char *text) {
    if (!strstr(output, text)) {
        fprintf(stderr, "missing combined output: %s", text);
        failures++;
    }
}

static void expect_absent(const char *output, const char *text) {
    if (strstr(output, text)) {
        fprintf(stderr, "unexpected combined output: %s", text);
        failures++;
    }
}

int main(void) {
    const char *input =
        "withdrawn01:\n"
        "   load r1,7\n"
        "   icopy r2,r1\n"
        "   unlink r2\n"
        "case01:\n"
        "   load r1,8\n"
        "   icopy r2,r1\n"
        "   .traceevent \"A\" 6 \"R\" \"I\" \"r\" 2 1 1 0 \"wide_alias\" \"\"\n"
        "   unlink r2\n"
        "   unlink r3\n"
        "case02:\n"
        "   icopy r2,r4\n"
        "   .traceevent \"A\" 6 \"R\" \"I\" \"r\" 2 1 1 0 \"multi_alias\" \"\"\n"
        "   unlink r2\n"
        "   unlink r3\n"
        "case03:\n"
        "   setattrs r1,6\n"
        "   iadd r2,r3,1\n"
        "   linkattr1 r4,r1,r2\n"
        "case04:\n"
        "   linkattr1 r1,r2,3\n"
        "   icopy r1,r4\n"
        "   .traceevent \"A\" 6 \"R\" \"I\" \"r\" 1 1 1 0 \"v\" \"\"\n"
        "   unlink r1\n"
        "case05:\n"
        "   setattrs r1,6\n"
        "   linkattr1 r2,r1,r3\n"
        "   load r4,9\n"
        "case06:\n"
        "   linkattr1 r1,r2,3\n"
        "   setattrs r1,6\n"
        "   iadd r4,r5,1\n"
        "   linkattr1 r6,r1,r4\n"
        "case07:\n"
        "   icopy r1,r2\n"
        "   .traceevent \"A\" 6 \"R\" \"I\" \"r\" 1 1 1 0 \"single_alias\" \"\"\n"
        "   unlink r1\n"
        "case08:\n"
        "   icopy r1,r2\n"
        "   unlink r2\n"
        "case09:\n"
        "   unlink r1\n"
        "   unlink r2\n"
        "case10:\n"
        "   unlink r1\n"
        "   br done10\n"
        "case11:\n"
        "   minattrs r1,r2,1\n"
        "   linkattr1 r3,r1,r2\n"
        "case12:\n"
        "   minattrs r1,3\n"
        "   linkattr1 r2,r1,3\n"
        "case13:\n"
        "   setattrs r1,6\n"
        "   linkattr1 r2,r1,r3\n"
        "generic_itof_stays_expanded:\n"
        "   icopy r1,r2\n"
        "   itof r1\n"
        "measured_arithmetic_chain:\n"
        "   icopy r1,r2\n"
        "   itof r1\n"
        "   fmult r1,r1,2.0\n"
        "   icopy r3,r4\n"
        "   itof r3\n"
        "case14:\n"
        "   fdiv r1,r2,r1\n"
        "   fsub r3,r1,1.5\n"
        "case15:\n"
        "   fmult r1,r1,2.0\n"
        "   icopy r2,r3\n"
        "promote_link_setlink:\n"
        "   linkattr1 r1,r2,3\n"
        "   setlinkattr1 r6,r1,6,r5,1\n"
        "promote_setlink_load:\n"
        "   setlinkattr1 r2,r1,6,r3\n"
        "   load r4,11\n"
        "negative01:\n"
        "   fsub r1,r2,1.0\n"
        "   .srcstep 1 1 17 \"x.crexx\" 1 1 1 \"x\"\n"
        "   load r3,14\n";
    char *output = rxcp_combine_superinstructions(input);

    if (!output) return 2;

    expect_present(output, "   load r1,7\n   isetunlink r2,r1\n");
    expect_absent(output, "   iloadsetunlink r1,r2,7\n");
    expect_present(output, "   iloadsetunlinkn r1,r2,8,r3\n");
    expect_present(output, "\"r\" 1 1 1 0 \"wide_alias\"");
    expect_present(output, "   isetunlinkn r2,r4,r3\n");
    expect_present(output, "\"r\" 4 1 1 0 \"multi_alias\"");
    expect_present(output, "   setlinkattr1 r4,r1,6,r3,1\n");
    expect_present(output, "   isetattr1 r2,3,r4\n");
    expect_present(output, "\"r\" 4 1 1 0 \"v\"");
    expect_present(output, "   setlinkiload r2,r1,6,r3,r4,9\n");
    expect_present(output, "   linksetattrslinkadd r1,r2,3,6,r6,r5,1\n");
    expect_present(output, "   isetunlink r1,r2\n");
    expect_present(output, "\"r\" 2 1 1 0 \"single_alias\"");
    expect_present(output, "   igetunlink r1,r2\n");
    expect_present(output, "   unlinkn r1,r2\n");
    expect_present(output, "   unlinkbr r1,done10\n");
    expect_present(output, "   minlinkattr1 r3,r1,r2,1\n");
    expect_present(output, "   minlinkattr1 r2,r1,3\n");
    expect_present(output, "   setlinkattr1 r2,r1,6,r3\n");

    expect_present(output, "generic_itof_stays_expanded:\n   icopy r1,r2\n   itof r1\n");
    expect_present(output, "measured_arithmetic_chain:\n   itof r1,r2\n   fmulticopy r1,2.0,r3,r4\n   itof r3\n");
    expect_present(output, "   fdivsub r3,r2,r1,1.5\n");
    expect_present(output, "   fmulticopy r1,2.0,r2,r3\n");
    expect_present(output, "promote_link_setlink:\n   linksetattrslinkadd r1,r2,3,6,r6,r5,1\n");
    expect_present(output, "promote_setlink_load:\n   setlinkiload r2,r1,6,r3,r4,11\n");
    expect_present(output, "   fsub r1,r2,1.0\n   .srcstep");
    expect_present(output, "   load r3,14\n");

    free(output);
    if (failures) return 1;
    puts("rxcp superinstruction combiner: pass");
    return 0;
}
