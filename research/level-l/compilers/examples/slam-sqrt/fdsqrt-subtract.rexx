/* ANALYSIS ONLY: revised FDSqrt with Newton addition changed to subtraction.
 * Same local names and layout; the normalized tree must distinguish it. */
FDSqrt: procedure
  parse arg x
  if x<=0 then return 0
  guess=x; if guess<1 then guess=1
  do forever
    next=(guess-(x/guess))/2
    if next>=guess then leave
    guess=next
  end
  return guess
