/* Example usage */
arrays.1.0 = 2
arrays.1.1 = "A"
arrays.1.2 = "B"

arrays.2.0 = 2
arrays.2.1 = "1"
arrays.2.2 = "2"

arrays.3.0 = 3
arrays.3.1 = "X"
arrays.3.2 = "Y"
arrays.3.3 = "Z"

arrays.0 = 3  /* Number of arrays */

call combine 1, ""
exit

/* REXX Implementation */
combine: procedure expose arrays.
  parse arg currentLevel, currentCombination

  if currentLevel > arrays.0 then do
    say currentCombination
    return
  end

  do i = 1 to arrays.currentLevel.0
    nextCombination = currentCombination || arrays.currentLevel.i
    call combine currentLevel + 1, nextCombination
  end
  return
