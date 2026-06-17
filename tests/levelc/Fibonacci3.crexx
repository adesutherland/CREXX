/* Rexx version of Nelson H. F. Beebe's Fibonacci example.        */
/* See: http://www.math.utah.edu/~beebe/software/java/fibonacci/  */
/*                                                 mfc 2002.06.10 */
options levelb
/* ----------------------------------------------------------------- */
/* Print ascending members of the Fibonacci sequence that are */
/* representable as 64-bit signed integers, prefixed by their term */
/* numbers, and followed by the ratio of successive terms, to */
/* demonstrate the 1.618...^n growth (the ratio approaches the */
/* golden ratio, (1 + sqrt(5))/2 = 1.6180339887498949.  The fourth */
/* item on each line is the difference from the golden ratio). */
/* ------------------------------------------------------------------ */

/* Rexx supports arbitrary precision floating-point arithmetic. */
/* For comparison with other programming languages, we limit it to */
/* work with to 20 digits of precision. */

/* sqrt is not in all Rexx distributions so we use a constant for */
/* the Golden Ratio. */


/* numeric digits 20 */

golden_ratio=1.6180339887498948482

limit=2**63-1
hi=1
lo=1
n=1
tab='\t'

say n tab lo

do while hi<limit
  n=n+1
  ratio=hi/lo

  say n tab hi tab ratio tab ratio-golden_ratio

  hi=lo+hi
  lo=hi-lo
end
