/* lcm_gcd.rexx
   The least common multiple (LCM) is the smallest positive
   integer that is divisible by both numbers.

   It can be computed efficiently using the relationship:

      LCM(a,b) = a * b / GCD(a,b)

   where GCD is the greatest common divisor.
*/
options levelb

say lcm(1071, 462)      /* 23562 */
say lcm(81, 49)         /* 3969  */

return

lcm: procedure = .int
  arg a = .int, b = .int

  /* LCM(a,b) = a * b / GCD(a,b) */
return a * b / gcd(a, b)

gcd: procedure = .int
  arg a = .int, b = .int

  /* Euclid's algorithm:
     repeatedly replace (a,b) with (b, a mod b)
     until the remainder becomes zero. */
  do while b <> 0
     t = a % b          /* Remainder of a divided by b */
     a = b              /* Shift divisor to a */
     b = t              /* Continue with the remainder */
  end

return a                /* Final non-zero value is the GCD */