/*
 * ooRexx object port of the Are We Fast Yet? List benchmark.
 * Derived from the SOM benchmark suite under the MIT license; see
 * ../../LICENSE-SOM-MIT.txt and ../../README.md.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=0
do iteration=1 to repetitions
  benchmark=.ListBenchmark~new
  result=benchmark~run
  if result<>10 then call fail 'expected length 10, got' result
end

say 'benchmark=awfy_list repetitions='repetitions 'result='result
say 'PASS: AWFY List ooRexx object port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::class ListElement
::method init
  expose value nextElement
  use strict arg value
  nextElement=.nil

::attribute nextElement

::method length
  expose nextElement
  if nextElement==.nil then return 1
  return 1+nextElement~length

::class ListBenchmark
::method run
  result=self~tail(self~makeList(15),self~makeList(10),self~makeList(6))
  return result~length

::method makeList private
  use strict arg listLength
  if listLength=0 then return .nil
  element=.ListElement~new(listLength)
  element~nextElement=self~makeList(listLength-1)
  return element

::method isShorterThan private
  use strict arg x,y
  xTail=x
  yTail=y
  do while yTail\==.nil
    if xTail==.nil then return .true
    xTail=xTail~nextElement
    yTail=yTail~nextElement
  end
  return .false

::method tail private
  use strict arg x,y,z
  if self~isShorterThan(y,x) then
    return self~tail(self~tail(x~nextElement,y,z),self~tail(y~nextElement,z,x),self~tail(z~nextElement,x,y))
  return z
