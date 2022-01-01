/* rexx */
options levelb
/* VERIFY */
say "Look for VERIFY OK"
/* These from the Rexx book. */
if verify('123','1234567890') \= 0 then say 'failed in test 1 '
if verify('1Z3','1234567890') \= 2 then say 'failed in test 2 '
if verify('AB4T','1234567890','M') \= 3 then say 'failed in test 3 '
if verify('1P3Q4','1234567890',,3) \= 4 then say 'failed in test 4 '
if verify('ABCDE','',,3) \= 3 then say 'failed in test 5 '
if verify('AB3CD5','1234567890','M',4) \= 6 then say 'failed in test 6 '
/* These from Mark Hessling. */
if verify('foobar', 'barfo', N, 1) \= 0 then say 'failed in test 7 '
if verify('foobar', 'barfo', M, 1) \= 1 then say 'failed in test 8 '
if verify('', 'barfo') \= 0 then say 'failed in test 9 '
if verify('foobar', '') \= 1 then say 'failed in test 10 '
if verify('foobar', 'barf', N, 3) \= 3 then say 'failed in test 11 '
if verify('foobar', 'barf', N, 4) \= 0 then say 'failed in test 12 '
if verify('', '') \= 0 then say 'failed in test 13 '
say "VERIFY OK"

/* say verify('123456','0123456789') */
/* say verify('12w36','0123456789') */
/* say verify('12w36','0123456789',,4) */
/* say verify('a12w36','0123456789','M') */

/* return */

/* verify()  */
verify: procedure = .int
  arg instring = .string, intab = .string, match = 'N',pos=1
