/* cRexx version of Nelson H. F. Beebe's Fibonacci example.        */
/* See: http://www.math.utah.edu/~beebe/software/java/fibonacci/  */
/*                                                 mfc 2002.06.10 */

options levelb

golden_ratio=1.6180339887498948482

limit=2**63-1            /* when to stop */
hi=1                     /* the integers */
lo=1                     /* .. */
n=1                      /* counter */
tab='\t'                 /* for output like the other samples */

say n tab lo

do hi=1 to limit
  n = n + 1
  ratio = hi / lo
  say n tab hi tab ratio tab ratio-golden_ratio
  hi=lo+hi
  lo=hi-lo
end