options levelb

main: procedure = .int
  say "Starting short circuit inline test..."
  calls = 0

  if 0 & probe(1, calls) then say "bad-and-left"
  else say "and-left-ok"
  say calls

  if 1 | probe(0, calls) then say "or-left-ok"
  else say "bad-or-left"
  say calls

  if \probe(0, calls) then say "not-ok"
  else say "bad-not"
  say calls

  if 1 & probe(1, calls) then say "and-right-ok"
  else say "bad-and-right"
  say calls

  if 0 | probe(1, calls) then say "or-right-ok"
  else say "bad-or-right"
  say calls

  say "Short circuit inline test finished."
  return 0

probe: procedure = .int
  arg flag = .int, expose count = .int
  count = count + 1
  return flag
