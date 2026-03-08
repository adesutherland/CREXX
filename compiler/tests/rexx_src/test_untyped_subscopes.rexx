options levelb

main: procedure
  x = 1
  do
    x = 2 /* Should update outer x if it's not a subscope for untyped */
    y = 3 /* New untyped variable */
    say "Inside do block: x =" x "y =" y
  end
  say "Outside do block: x =" x "y =" y
  
  if x = 2 & y = 3 then say "Untyped variables are procedure-scoped (Normal REXX)"
  else say "Untyped variables are DO-block-scoped"
  
  return
