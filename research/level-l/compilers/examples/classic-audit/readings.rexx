parse arg readings                    /* Read a space-separated list of measurements. */
remaining = words(readings)            /* Count the measurements to examine. */
accepted = 0                          /* Count usable measurements separately. */
total = 0                             /* Start their sum at zero. */
do while remaining > 0                /* Visit the list from its last word to its first. */
  value = word(readings, remaining)    /* Read the current measurement. */
  remaining = remaining - 1           /* Make progress even when this measurement is rejected. */
  if datatype(value, 'N') = 0 then iterate /* Skip text that is not a number. */
  if value < 0 then iterate           /* Skip negative measurements. */
  total = total + value               /* Add a usable measurement. */
  accepted = accepted + 1             /* Count it in the average. */
end                                  /* Examine the next measurement. */
if accepted = 0 then return 'no valid readings' /* Handle a list with no usable measurements. */
return total / accepted               /* Average only the measurements we accepted. */
