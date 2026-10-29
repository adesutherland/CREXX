/* ANALYSIS ONLY: old FDSqrt with divisor 2 changed to literal zero.
 * A consumer tree rule detects this fault before execution. */
FDSqrt: procedure
  parse arg x
  if x<=0 then return 0
  guess=x; if guess<1 then guess=1
  do 14; guess=(guess+(x/guess))/0; end
  return guess
