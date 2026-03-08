options levelb
main: procedure
  x = 1
  do
     x = 2
     y = 3
     do
        x = 4
        y = 5
        z = 6
        say "Inside 2 (expect 4 5 6): x =" x "y =" y "z =" z
     end
     say "Inside 1 (expect 4 5 z): x =" x "y =" y "z =" z
  end
  say "Outside (expect 4 y z): x =" x "y =" y "z =" z
  
  if x = 4 & y = "Y" & z = "Z" then say "SUCCESS: Untyped subscoping works correctly"
  else say "FAILURE: Scoping failed, x =" x "y =" y "z =" z
  
  return
