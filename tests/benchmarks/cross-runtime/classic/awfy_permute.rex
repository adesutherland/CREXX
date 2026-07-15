/*
 * Classic Rexx procedural port of the Are We Fast Yet? Permute benchmark.
 * Derived from the SOM benchmark suite under the MIT license; see
 * ../../LICENSE-SOM-MIT.txt and ../../README.md.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=0
do iteration=1 to repetitions
  count=0
  values.=0
  call permute 6
  result=count
  if result<>8660 then call fail 'expected 8660 calls, got' result
end

say 'benchmark=awfy_permute repetitions='repetitions 'result='result
say 'PASS: AWFY Permute Classic procedural port'
exit 0

permute: procedure expose count values.
  parse arg n
  count=count+1
  if n<>0 then do
    n1=n-1
    call permute n1
    do i=n1 to 0 by -1
      call swap n1,i
      call permute n1
      call swap n1,i
    end
  end
  return

swap: procedure expose values.
  parse arg i,j
  temporary=values.i
  values.i=values.j
  values.j=temporary
  return

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1
