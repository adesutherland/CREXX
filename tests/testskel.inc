/* cREXX Assembler Test Suite include skeleton */

.globals=2
g0 .expose=tests.success
g1 .expose=tests.failure

logo()         .locals=2
    say "cREXX Assembler Test Suite - Common - Version cREXX-Phase-0c"
    
    * Zero result count
    load g0,0
    load g1,0
    ret

failures:      /* There were failures */
    itos g1
    load r0,"***"
    sconcat r1,r0,g1
    load r0,"tests failed ***"
    sconcat r1,r1,r0
    say r1
    ret 1

expect_true()  .locals=0
    brt expect_true_ok,a1
    iadd g1,g1,1  /* Failure */
    say " FAILED"
    ret
expect_true_ok:
    iadd g0,g0,1  /* Success */
    say " ok"
    ret
    
expect_false() .locals=0
    brf expect_false_ok,a1
    iadd g1,g1,1  /* Failure */
    say " FAILED"
    ret
expect_false_ok:
    iadd g0,g0,1 /* Success */
    say " ok"
    ret


    /* Print Results */
print_results() .locals=3
    iadd r0, g0, g1
    itos r0
    load r1,"tests run"
    sconcat r1,r0,r1
    say r1
    brt failures, g1
    
    /* No failures */
    say "Success"
    ret 0
