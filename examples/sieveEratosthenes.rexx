/* sieve.crx
   The Sieve of Eratosthenes finds all prime numbers up to
   a given limit by repeatedly marking multiples of each prime.
*/
options levelb
import rxfnsb
import system

arg parms = .string[]
if parms[1] = '' then limit = 1000
else limit = parms[1]

/* Assume all numbers are prime initially */
isprime = .int[]
do i = 1 to limit
   isprime[i] = 1
end

isprime[1] = 0              /* 1 is not a prime number */

/* For each prime p, mark all multiples of p as composite */
do p = 2 while p * p <= limit
   if isprime[p] then do
      do n = p * p to limit by p
         isprime[n] = 0
      end
   end
end

/* Print all numbers that remain marked as prime */
do i = 2 to limit
   if isprime[i] then say i
end