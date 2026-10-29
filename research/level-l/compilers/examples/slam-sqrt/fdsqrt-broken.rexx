/* Teaching fault: NEXT is assigned only when X > 1.
 * Adapted from the FDSqrt source credited in README.md. */
FDSqrt: procedure
  parse arg x
  if x<=0 then return 0
  guess=x; if guess<1 then guess=1
  do forever
    if x>1 then next=(guess+(x/guess))/2
    if next>=guess then leave
    guess=next
  end
  return guess
