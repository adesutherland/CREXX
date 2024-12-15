/* rexx test abs bif */
options levelb
import rxfnsb

/* TODO RXVM ts_time time trunc right left _elapsed copies length substr */
say "test Time"
say "Option 'E' "time('E')
/* instead of a wait we run a large loop */
do i=1 to 1000000
   a=i
end
say "Option 'E' "time('E')
say "Option 'R' "time('R')
say "Option 'E' "time('E')
/* instead of a wait we run a large loop */
do i=1 to 2500000
   a=i
end
say "Option 'E' "time('E')

say "Option '' "time()
say "Option N  "time("N")
say "Option L  "time("l")
say "Option C  "time("C")
say "Option H  "time("h")
say "Option M  "time("M")
say "Option S  "time("s")
say "Option US "time("us")  /* microseconds since midnight */


say "Option ZD  "time("zd")   /* time differnce to UTC in seconds */
say "Option T   "time("T")    /* cpu ticks since program start    */
say "Option TS  "time("TS")   /* cpu ticks per seconds            */
say "Option ZN  "time("zN")   /* time zone name ; summer time zone name (if any) */
say "Option UTC "time("uTc")  /* UTC time */

return 0
