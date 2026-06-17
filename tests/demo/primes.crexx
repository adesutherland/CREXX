#!/usr/local/crexx/rexx.sh
/* Create Prime Numbers */
options levelb
import rxfnsb

generate_primes = 10

prime = .int[]

do p = 2 until prime.0 >= generate_primes
   is_prime = 1
   do i = 1 to prime.0 while is_prime = 1
     if prime.i ** 2 > p then leave i
     if p // prime.i = 0 then is_prime = 0
   end
   if is_prime = 1 then do
        prime[prime.0 + 1] = p
   end
end

/* address cmd "rm primes.txt" */
do i = 1 to prime.0
  say prime.i
  /* call lineout "primes.txt", prime.i */
end
/* call lineout "primes.txt" */



