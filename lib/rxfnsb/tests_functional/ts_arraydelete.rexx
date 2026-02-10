/* rexx test arraydelete bif */
options levelb
import rxfnsb

errors=0
array=.string[]
/* ---------------------------------------------------------------
 * Test script for ARRAYDELETE(array, from, count)
 * --------------------------------------------------------------- */

/* ---------- test harness ---------- */

say "=== ARRAYDELETE() tests ==="
say
call test "1) delete middle", "A B C D E F", 3, 2, "A B E F"
call test "2) delete first element", "A B C D", 1, 1, "B C D"
call test "3) delete last element", "A B C D", 4, 1, "A B C"
call test "4) delete entire array", "A B C", 1, 10, ""
call test "5) delete beyond end (no-op)", "A B C", 5, 2, "A B C"
call test "6) delete with count=0 (no-op)", "A B C", 2, 0, "A B C"
call test "7) delete with from<1 (no-op)", "A B C", 0, 2, "A B C"
call test "8) delete tail", "A B C D E", 3, 10, "A B"
call test "9) delete single middle", "A B C D E", 3, 1, "A B D E"
say
say "Done."
exit 0


/* ---------- helpers ---------- */

test: procedure
  arg label=.string, data=.string, from=.int, count=.int, expected=.string

  a = .string[]
  do i = 1 to words(data)
     a[i] = word(data, i)
  end
  call arraydelete a, from, count
  got = join(a)

  if got = expected then say "OK   - "label
  else do
    say "FAIL - "label
    say "       from="from "count="count
    say "       expect='"expected"'"
    say "       got   ='"got"'"
  end
return


join: procedure=.string
  arg a=.string[]
  out = ""
  do i = 1 to a[0]
    out = out || a[i]
    if i < a[0] then out= out || " "
  end
return out



