/* Controlled variation: old FDSqrt with iteration count 14 changed to 1.
 * This is deliberately inaccurate; attribution is in README.md. */
FDSqrt: procedure
  parse arg x
  if x<=0 then return 0
  guess=x; if guess<1 then guess=1
  do 1; guess=(guess+(x/guess))/2; end
  return guess
