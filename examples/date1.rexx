/* date_today.rexx
   The DATE() function returns today's date in different formats
   and can also convert a given date into alternative representations.
*/
options levelb
import rxfnsb

/* Today's date in various formats */
say 'Normal :' date()        /* Default human-readable format */
say 'Base   :' date('B')     /* Days since 0001-01-01 */
say 'Julian :' date('J')     /* YYYYDDD (year + day-of-year) */
say 'Unix   :' date('U')     /* Days since 1970-01-01 */

/* Equivalent calls using explicit format options */
say date('N')                /* Normal format */
say date('B')                /* Base date number */
say date('J')                /* Julian format */
say date('U')                /* Unix date number */

d = '2026-05-11'

/* Convert a specific input date.
   'INT' indicates an ISO-style input date (YYYY-MM-DD),
   '-' is the input separator. */
say 'Input :' d
say 'Normal:' date('N', d, 'INT', , '-')   /* Convert to normal format */
say 'Base  :' date('B', d, 'INT', , '-')   /* Convert to base days */
say 'Unix  :' date('U', d, 'INT', , '-')   /* Convert to Unix days */