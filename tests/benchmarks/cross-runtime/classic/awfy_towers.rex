/*
 * ooRexx object port of the Are We Fast Yet? Towers benchmark.
 * Derived from the SOM benchmark suite under the MIT license; see
 * ../../LICENSE-SOM-MIT.txt and ../../README.md.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=0
do iteration=1 to repetitions
  benchmark=.TowersBenchmark~new
  result=benchmark~run
  if result<>8191 then call fail 'expected 8191 moves, got' result
end

say 'benchmark=awfy_towers repetitions='repetitions 'result='result
say 'PASS: AWFY Towers ooRexx object port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::class TowersDisk
::method init
  expose diskSize nextDisk hasNextValue
  use strict arg diskSize
  nextDisk=.nil
  hasNextValue=.false

::method size
  expose diskSize
  return diskSize

::method next
  expose nextDisk
  return nextDisk

::method hasNext
  expose hasNextValue
  return hasNextValue

::method setNext
  expose nextDisk hasNextValue
  use strict arg value
  nextDisk=value
  hasNextValue=.true

::class TowersBenchmark
::method init
  expose pile0 pile1 pile2 pile0Present pile1Present pile2Present movesDone
  pile0=.nil
  pile1=.nil
  pile2=.nil
  pile0Present=.false
  pile1Present=.false
  pile2Present=.false
  movesDone=0

::method run
  expose movesDone
  self~buildTowerAt(0,13)
  movesDone=0
  self~moveDisks(13,0,1)
  return movesDone

::method pushDisk private
  expose pile0 pile1 pile2 pile0Present pile1Present pile2Present
  use strict arg disk,pile
  top=.nil
  hasTop=.false
  select
    when pile=0 then do
      hasTop=pile0Present
      if hasTop then top=pile0
    end
    when pile=1 then do
      hasTop=pile1Present
      if hasTop then top=pile1
    end
    otherwise do
      hasTop=pile2Present
      if hasTop then top=pile2
    end
  end
  if hasTop then do
    if disk~size>=top~size then return
    disk~setNext(top)
  end
  select
    when pile=0 then do
      pile0=disk
      pile0Present=.true
    end
    when pile=1 then do
      pile1=disk
      pile1Present=.true
    end
    otherwise do
      pile2=disk
      pile2Present=.true
    end
  end

::method popDiskFrom private
  expose pile0 pile1 pile2 pile0Present pile1Present pile2Present
  use strict arg pile
  select
    when pile=0 then top=pile0
    when pile=1 then top=pile1
    otherwise top=pile2
  end
  if top~hasNext then do
    nextDisk=top~next
    select
      when pile=0 then pile0=nextDisk
      when pile=1 then pile1=nextDisk
      otherwise pile2=nextDisk
    end
  end
  else do
    select
      when pile=0 then pile0Present=.false
      when pile=1 then pile1Present=.false
      otherwise pile2Present=.false
    end
  end
  return top

::method moveTopDisk private
  expose movesDone
  use strict arg fromPile,toPile
  disk=self~popDiskFrom(fromPile)
  self~pushDisk(disk,toPile)
  movesDone=movesDone+1

::method buildTowerAt private
  use strict arg pile,disks
  do i=disks to 0 by -1
    disk=.TowersDisk~new(i)
    self~pushDisk(disk,pile)
  end

::method moveDisks private
  use strict arg disks,fromPile,toPile
  if disks=1 then self~moveTopDisk(fromPile,toPile)
  else do
    otherPile=(3-fromPile)-toPile
    self~moveDisks(disks-1,fromPile,otherPile)
    self~moveTopDisk(fromPile,toPile)
    self~moveDisks(disks-1,otherPile,toPile)
  end
