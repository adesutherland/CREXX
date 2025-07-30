/* --------------------------------------------------------------------------
 * Contains Mathematical REXX Procedures, can be added with ##USE SYSLIB.REXX
 * --------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------
 * Returns the greatest common divisor (GCD) of two numbers
 * --------------------------------------------------------------------------
 */
gcd: procedure=.int
  arg a=.int, b=.int
  do while b \= 0
     t = b
     b = a // b
     a = t
  end
return a
/* --------------------------------------------------------------------------
 * Returns the least common multiple (LCM) of two numbers
 * --------------------------------------------------------------------------
 */
lcm: procedure=.int
  arg a=.int, b=.int
return (a * b) % gcd(a, b)
/* --------------------------------------------------------------------------
 * Returns 1 if n is a prime number, 0 otherwise
 * --------------------------------------------------------------------------
 */
isPrime: procedure=.int
  arg n=.int
  if n <= 1 then return 0
  if n = 2 then return 1
  if n // 2 = 0 then return 0
  do i = 3 to n % 2 by 2
     if n // i = 0 then return 0
  end
return 1
/* --------------------------------------------------------------------------
 * Returns the factorial of n (n!)
 * --------------------------------------------------------------------------
 */
factorial: procedure=.int
  arg n=.int
  if n < 0 then return -1
  f = 1
  do i = 2 to n
     f = f * i
  end
return f
/* --------------------------------------------------------------------------
 * Returns base raised to the power exp (base^exp)
 * --------------------------------------------------------------------------
 */
pow: procedure=.float
  arg base=.float, exp=.float
return base**exp
/* --------------------------------------------------------------------------
 * Returns the modular inverse of a modulo m, or -1 if not possible
 * --------------------------------------------------------------------------
 */
 modinv: procedure=.int
  arg a=.int, m=.int
  t = 0
  newt = 1
  r = m
  newr = a
  do while newr \= 0
     q = r // newr
     temp = newt
     newt = t - q * newt
     t = temp
     temp = newr
     newr = r - q * newr
     r = temp
  end
  if r > 1 then return -1
  if t < 0 then t = t + m
return t