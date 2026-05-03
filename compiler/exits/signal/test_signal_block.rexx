options levelb comments_dash

main: procedure
  failures = 0
  handled_name = ""
  handled_message = ""
  nested_message = ""
  catch_all_hit = 0
  no_as_hit = 0

  do
    signal other "block payload"
    failures = failures + 1
  on signal other as problem
    handled_name = problem.name()
    handled_message = problem.message()
  end

  if handled_name \= "OTHER" then failures = failures + 1
  if handled_message \= "block payload" then failures = failures + 1

  do
    call raise_other
    failures = failures + 1
  on signal other as problem
    nested_message = problem.message()
  end

  if nested_message \= "nested payload" then failures = failures + 1

  do
    signal conversion_error "catch all payload"
    failures = failures + 1
  on signal
    catch_all_hit = 1
  end

  if catch_all_hit \= 1 then failures = failures + 1

  do
    signal other "no as payload"
    failures = failures + 1
  on signal other
    no_as_hit = 1
  end

  if no_as_hit \= 1 then failures = failures + 1

  if failures = 0 then say "SUCCESS"
  else say "FAIL"
  return

raise_other: procedure
  signal other "nested payload"
  return
