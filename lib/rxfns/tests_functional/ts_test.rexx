/* rexx test abs bif */
options levelb
/* <<<<<<< HEAD */
/* say translate(ABCDEF,1e+17,'ac','1') "\= '1bEdef '" */
/* return */

/* translate: procedure = .string */
/* arg source = .string, tochar = "?????", fromchar = "?????", pad=" " */
/* ======= */
/* TODO */
a="This is René's test case   "
z="     This is René's test case"
say '"Test case A "'a'"'
say '"Test case B "'z'"'
x=0
/* Test strict comparison REG/REG */
if a=z then say "equal (strict compare)"
   else say "not Equal (strict compare)"
/* Test non strict comparison REG/REG */
assembler rseq x,a,z
if x=1 then say "EQUAL (non strict compare)"
   else say "NOT EQUAL (non strict compare)"

return 0
/* >>>>>>> develop */
