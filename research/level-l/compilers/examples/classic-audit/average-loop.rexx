parse arg total, count                  /* Read an initial total and count. */
if count = 0 then return 'no items'      /* Check the count once, before the loop. */
do while total > 0                     /* Keep calculating while the total is positive. */
  average = total / count              /* Later iterations can use a different count. */
  count = count - 1                    /* With count = 1, the next count becomes zero. */
  total = total - 1                    /* Move towards the loop's exit. */
end                                   /* Repeat without rechecking count. */
return average                        /* Return the last result. */
