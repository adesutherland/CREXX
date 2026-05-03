/* Create Prime Numbers - Classic Rexx */

generate_primes = 100000

n = 0

do p = 2 until n >= generate_primes
   is_prime = 1
   do i = 1 to n while is_prime = 1
     if prime.i ** 2 > p then leave i
     if p // prime.i = 0 then is_prime = 0
   end
   if is_prime = 1 then do
        n = n + 1
        prime.n = p
   end
end
prime.0 = n

address cmd "rm classicprimes.txt"
do i = 1 to prime.0
  call lineout "classicprimes.txt", prime.i
end
call lineout "classicprimes.txt"
