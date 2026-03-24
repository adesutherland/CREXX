/* Test for Optimizer Decimal Safety */
options levelb

call Case1
call Case2
call Case3
call TestInherited
return

Case1: procedure
    numeric digits 5
    say 1.0d/3
    return

Case2: procedure
    numeric digits 9
    say 1.0d/3
    return

Case3: procedure
    numeric digits 9
    numeric fuzz 1
    if 1.00000001 = 1.0 then say "Equal with Fuzz"
    else say "Not Equal"
    return

TestInherited: procedure
    /* Inherited context: Compiler should treat digits as unknown (-1) 
       and NOT fold this calculation. It should generate a divide instruction. */
    say 1.0d/3
    return
