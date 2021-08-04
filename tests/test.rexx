/* Tests for latest show and tells */
options levelb

say "Début"

x = 4
/*
x = 4 /* Fool the optimiser */
*/

a = 3 + 1        /* 4 */

if x = 4 | a = 5 then say true

f1 = 1.0
f2 = 1.1
if f1=1 then say "1.0 = 1"

s1 = "left"
if s1 <> "right" then say "left is not right"

b = a / 2        /* 2 */
c = b + a + 4    /* 10 */
d = a + b + c    /* 16 */

e = 8 + 8        /* 16 */

if d=e then say "success - got" d "expecting" e
else say "failure - got" d "expecting" e

