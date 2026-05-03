#!/usr/local/crexx/rexx.sh
/* Pass Environment Variables */
options levelb
import rxfnsb

out = .string[]

userid = "Demo's userid"

address cmd "penv.sh" output out expose userid blah
if rc <> 0 then say "RC = "rc "when doing : penv.sh"

say "penv.sh output:"
do i = 1 to out.0
    say ">>>>" out.i
end