parse arg total, count                  /* Read the total and number of items. */
if count = 0 then return 'no items'      /* Check the original count. */
count = count - 1                       /* Exclude one item; count may now be zero. */
return total / count                   /* The earlier check no longer protects this division. */
