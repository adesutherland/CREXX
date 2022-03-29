/* rexx test abs bif */
options levelb
/* TODO */
a="This is René's test case   "
b="     This is René's test case"
say '"Test case A "'a'"'
say '"Test case B "'b'"'
x=0
/* Test strict comparison REG/REG */
if a=b then say "equal (strict compare)"
   else say "not Equal (strict compare)"
/* Test non strict comparison REG/REG */
assembler rseq x,a,b
if x=1 then say "EQUAL (non strict compare)"
   else say "NOT EQUAL (non strict compare)"

return 0
