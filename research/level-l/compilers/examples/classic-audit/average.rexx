parse arg total, count                  /* Read the total and number of items. */
if count = 0 then return 'no items'      /* Leave before dividing by zero. */
return total / count                   /* Calculate the average. */
