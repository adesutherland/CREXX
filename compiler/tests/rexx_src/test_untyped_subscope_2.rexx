options levelb

main: procedure
  do
    x = 2
    say "Inside: x =" x
  end
  say "Outside: x =" x
  
  if x = "x" | x = "X" then say "Untyped variables are DO-block-scoped"
  else say "Untyped variables are procedure-scoped (Classic REXX behavior)"
  
  return
