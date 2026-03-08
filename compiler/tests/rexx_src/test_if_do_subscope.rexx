options levelb
main: procedure
  x = 1
  if 1 then do
    x = .int
    x = 10
  end
  say "Outside: x =" x
  
  if x = 1 then say "SUCCESS: IF DO is a subscope"
  else say "FAILURE: IF DO is NOT a subscope"
  
  return
