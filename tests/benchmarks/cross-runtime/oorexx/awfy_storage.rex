/*
 * ooRexx object port of the Are We Fast Yet? Storage benchmark.
 * Derived from the SOM benchmark suite under the MIT license; see
 * ../../LICENSE-SOM-MIT.txt and ../../README.md.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=0
do iteration=1 to repetitions
  benchmark=.StorageBenchmark~new
  result=benchmark~run
  if result<>5461 then call fail 'expected 5461 allocations, got' result
end

say 'benchmark=awfy_storage repetitions='repetitions 'result='result
say 'PASS: AWFY Storage ooRexx object port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::class StorageRandom
::method init
  expose seedValue
  seedValue=74755

::method next
  expose seedValue
  seedValue=((seedValue*1309)+13849)//65536
  return seedValue

::class StorageBenchmark
::method run
  expose allocationCount
  random=.StorageRandom~new
  allocationCount=0
  tree=self~buildTreeDepth(7,random)
  if tree~items=0 then return 0
  return allocationCount

::method buildTreeDepth private
  expose allocationCount
  use strict arg depth,random
  allocationCount=allocationCount+1
  if depth=1 then return .array~new((random~next//10)+1)
  tree=.array~new(4)
  do i=1 to 4
    tree[i]=self~buildTreeDepth(depth-1,random)
  end
  return tree
