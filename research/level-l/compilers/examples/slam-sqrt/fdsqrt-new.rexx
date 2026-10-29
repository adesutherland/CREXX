/* Classic adaptation of FDSqrt after animatedads/alchemy 3761f57c.
 * Original door routine: ChatGPT; defect found by SLAM (Tom Dyer's design,
 * code by Claude, Opus 5.5). Only the ooRexx routine header/argument syntax
 * is adapted to Classic Rexx; the calculation is unchanged.
 */
FDSqrt: procedure
  parse arg x
  if x<=0 then return 0
  guess=x; if guess<1 then guess=1
  do forever
    next=(guess+(x/guess))/2
    if next>=guess then leave
    guess=next
  end
  return guess
