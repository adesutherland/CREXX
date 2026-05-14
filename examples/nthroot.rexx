/* nthroot.crx */
options levelb
import rxfnsb

say nthroot(2, 2)   /* sqrt(2) */
say nthroot(3, 2)   /* cbrt(2) */
say nthroot(4, 2)   /* 4th root of 2 */
say nthroot(2,81)   /* 2th root of 81 */
say nthroot(1.5,36) /* 1.5th root of 36 */

nthroot: procedure=.float
  arg n=.float, x=.float
  guess = x

  do 50
    guess = ((n - 1) * guess + x / guess**(n - 1)) / n
  end

return guess