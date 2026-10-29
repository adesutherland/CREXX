parse arg total, count                  /* Read the total and number of items. */
average = total / count                /* This division happens before the check. */
if count = 0 then return 'no items'      /* Checking here is too late. */
return average                        /* Return the calculated value. */
