parse arg remaining, skip              /* Read a counter and a flag for skipping work. */
do while remaining > 0                /* Repeat while work remains. */
  remaining = remaining - 1           /* Count this trip before any early restart. */
  if skip = 1 then iterate            /* Restart the loop after updating the counter. */
  say remaining                      /* Stand in for the useful work. */
end                                  /* Return to the loop test. */
return remaining                     /* Return the final counter. */
