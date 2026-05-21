/* time_demo.crx
   Demonstrates CREXX TIME() formats and elapsed-time measurement.
*/
options levelb
import rxfnsb

say 'Current Time Formats'
say '--------------------'
say 'Normal       :' time()        /* HH:MM:SS */
say 'Long         :' time('L')     /* HH:MM:SS.ffffff */
say 'Clock        :' time('C')     /* HH:MMam/pm */
say 'Hour         :' time('H')     /* Current hour */
say 'Minutes      :' time('M')     /* Minutes since midnight */
say 'Seconds      :' time('S')     /* Seconds since midnight */
say 'Microseconds :' time('US')    /* Microseconds since midnight */

say
say 'System Time'
say '-----------'
say 'UTC          :' time('UTC')   /* UTC time as HH:MM:SS */
say 'Timezone     :' time('ZN')    /* Time zone name */
say 'UTC offset   :' time('ZD')    /* Offset from UTC */
say 'Timestamp    :' time('TS')    /* Current timestamp */
