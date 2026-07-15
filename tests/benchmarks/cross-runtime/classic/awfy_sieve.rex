/*
 * Classic Rexx procedural port of the Are We Fast Yet? Sieve benchmark.
 * Derived from the SOM benchmark suite under the MIT license; see
 * ../../LICENSE-SOM-MIT.txt and ../../README.md.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=0
do iteration=1 to repetitions
  result=sieve_once()
  if result<>669 then call fail 'expected 669 primes, got' result
end

say 'benchmark=awfy_sieve repetitions='repetitions 'result='result
say 'PASS: AWFY Sieve Classic port'
exit 0

sieve_once: procedure
  flags.=1
  prime_count=0
  do i=2 to 5000
    if flags.i then do
      prime_count=prime_count+1
      do k=i+i to 5000 by i
        flags.k=0
      end
    end
  end
  return prime_count

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1
