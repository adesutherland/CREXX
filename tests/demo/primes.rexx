#!/usr/local/crexx/rexx.sh
/* Create Prime Numbers */
options levelb
import rxfnsb

generate_primes = 1000

primes = .int[]

do p = 2 until primes.0 >= generate_primes
   is_prime = 1
   do i = 1 to primes.0 while is_prime = 1
     if primes.i ** 2 > p then leave i
     if p // primes.i = 0 then is_prime = 0
   end
   if is_prime = 1 then do
        primes[primes.0 + 1] = p
   end
end

address cmd "rm primes.txt"
do i = 1 to primes.0
  call lineout "primes.txt", primes.i
end
call lineout "primes.txt"



