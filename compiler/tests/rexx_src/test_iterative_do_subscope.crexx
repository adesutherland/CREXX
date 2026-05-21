options levelb
main: procedure
  i = 0
  say "Before loop: i =" i
  do i = 1 to 5
     say "  Inside loop: i =" i
  end
  say "After loop: i =" i " (expect 6 in Classic REXX, but with subscope it might be different)"
  
  x = 1
  do j = 1 to 3
     x = .int
     x = j * 10
     say "    Nested typed x =" x
  end
  say "After loop: x =" x " (expect 1)"
  
  if x = 1 then say "SUCCESS: Iterative DO subscope works for typed vars"
  else say "FAILURE: Iterative DO subscope failed for typed vars, x =" x
  
  return
