/* Classic adaptation of animatedads/alchemy packages/oorexx_vision/src/Vision.cls
 * rx_vision_sqrt. The comparison driver admits only nonnegative numeric input;
 * ooRexx's negative-input RAISE is outside this Classic fixture.
 */
rx_vision_sqrt: procedure
  parse arg value
  if value=0 then return 0
  guess=value
  if guess<1 then guess=1
  do i=1 to 40
    next=(guess+value/guess)/2
    if abs(next-guess)<0.000000000001 then return next
    guess=next
  end
  return guess
