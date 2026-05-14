/* fibonacci.rexx
   The Fibonacci sequence starts with 0 and 1.
   Each following number is the sum of the two previous ones.
*/
options levelb
import rxfnsb

a = 0
b = 1

say 'Fibonacci Sequence'
say '------------------'

do i = 1 to 25
    say right(i,4,'0') right(a,8)   /* Print sequence number and value */

    t = a + b    /* Compute next Fibonacci number */
    a = b        /* Shift current value */
    b = t        /* Store next value */
end