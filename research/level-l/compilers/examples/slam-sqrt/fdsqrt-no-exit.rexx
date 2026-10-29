/* ANALYSIS ONLY: revised FDSqrt with its conditional LEAVE removed.
 * Never execute this control. Original attribution is in README.md. */
FDSqrt: procedure
  parse arg x
  if x<=0 then return 0
  guess=x; if guess<1 then guess=1
  do forever
    next=(guess+(x/guess))/2
    guess=next
  end
  return guess
