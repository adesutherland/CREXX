options levelb
/* Test Procedure Call */
say "Main Start"
call MyProc 10
say "Main End"
return

MyProc: procedure
  arg x = .int
  say "Inside Proc" x
  return
