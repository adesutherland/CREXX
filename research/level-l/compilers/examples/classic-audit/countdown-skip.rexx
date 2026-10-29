parse arg remaining, skip              /* Read a counter and a flag for skipping work. */
do while remaining > 0                /* Repeat while work remains. */
  if skip = 1 then iterate            /* Restart without updating the counter. */
  remaining = remaining - 1           /* This update can be skipped. */
  say remaining                      /* Stand in for the useful work. */
end                                  /* Return to the loop test. */
return remaining                     /* Return the final counter. */
