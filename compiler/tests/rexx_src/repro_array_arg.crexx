options levelb
import rxfnsb

s.1 = "first"
i = 1
say "Before: ["s.i"]"
/* upper() should not change the original register in-place if pass-by-value */
c = upper(s.i)
say "Upper:  ["c"]"
say "After:  ["s.i"]"

if s.i \= "first" then say "BUG: original stem element changed!"
else say "OK: stem element preserved"

return 0
