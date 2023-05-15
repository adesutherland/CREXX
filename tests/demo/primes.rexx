#!/usr/local/crexx/rexx.sh
/* Create Prime Numbers */
options levelb
import rxfnsb

generate_primes = 100

primes = .int[]

do p = 2 until primes.0 >= generate_primes
   is_prime = 1
   do i = 1 to primes.0 while is_prime = 1
     if primes.i * 2 > p then leave i
     if p // primes.i = 0 then is_prime = 0
   end
   if is_prime = 1 then do
        /* primes[primes.0 + 1] = p - TODO ASSEMBLER ERRORS */
        x = primes.0 + 1
        primes.x = p
   end
end

address cmd "rm primes.txt"
do i = 1 to primes.0
  /* TODO Parameter detection is broken if a variable is passed or for non-optional params */
  call lineout "primes.txt", i","primes.i
end
call lineout "primes.txt"



