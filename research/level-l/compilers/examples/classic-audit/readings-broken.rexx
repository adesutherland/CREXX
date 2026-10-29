parse arg readings                    /* Read a space-separated list of measurements. */
remaining = words(readings)            /* Count the measurements to examine. */
accepted = 0                          /* Count usable measurements separately. */
total = 0                             /* Start their sum at zero. */
do while remaining > 0                /* Visit the list from its last word to its first. */
  value = word(readings, remaining)    /* Read the current measurement. */
  if datatype(value, 'N') = 0 then iterate /* This restart skips the counter update. */
  if value < 0 then iterate           /* So does this one. */
  remaining = remaining - 1           /* Update only after accepting a measurement. */
  total = total + value               /* Add the measurement. */
  accepted = accepted + 1             /* Count it in the average. */
end                                  /* Examine the next measurement. */
return total / accepted               /* An empty list leaves accepted at zero. */
