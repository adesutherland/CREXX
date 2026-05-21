options levelb
/* Test Scope Hiding */
a = 50
call Inner
if a = 50 then say "Outer A Preserved"
else say "Outer A Corrupted"
return

Inner: procedure
  a = 10
  say "Inner A is" a
  return
