/* Classic adaptation of FDSqrt before animatedads/alchemy 3761f57c.
 * Original door routine: ChatGPT. SLAM architecture: Tom Dyer; SLAM code:
 * Claude (Opus 5.5). The ooRexx ::routine/use strict arg wrapper is replaced
 * with a Classic label/procedure/PARSE ARG; the calculation is unchanged.
 */
FDSqrt: procedure
  parse arg x
  if x<=0 then return 0
  guess=x; if guess<1 then guess=1
  do 14; guess=(guess+(x/guess))/2; end
  return guess
