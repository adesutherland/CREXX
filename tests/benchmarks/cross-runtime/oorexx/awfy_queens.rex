/*
 * ooRexx object port of the Are We Fast Yet? Queens benchmark.
 * Derived from upstream commit 74306fec151070fd07157cefeacf19e7e0bcdc89
 * under the SOM MIT license; see ../../THIRD_PARTY_NOTICES.md.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

result=.false
do iteration=1 to repetitions
  benchmark=.QueensBenchmark~new
  result=benchmark~run
  if \result then call fail 'eight-queens search did not find a placement'
end

say 'benchmark=awfy_queens repetitions='repetitions 'result='result
say 'PASS: AWFY Queens ooRexx object port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::class QueensBenchmark
::method init
  expose freeMaxs freeRows freeMins queenRows
  freeMaxs=.array~new(16)
  freeRows=.array~new(8)
  freeMins=.array~new(16)
  queenRows=.array~new(8)
  self~reset

::method reset private
  expose freeMaxs freeRows freeMins queenRows
  do i=1 to 16
    freeMaxs[i]=.true
    freeMins[i]=.true
  end
  do i=1 to 8
    freeRows[i]=.true
    queenRows[i]=-1
  end

::method run
  result=.true
  do i=1 to 10 while result
    result=self~queens
  end
  return result

::method queens private
  self~reset
  return self~placeQueen(0)

::method placeQueen private
  expose queenRows
  use strict arg column
  do row=0 to 7
    if self~rowColumnFree(row,column) then do
      queenRows[row+1]=column
      self~setRowColumn(row,column,.false)
      if column=7 then return .true
      if self~placeQueen(column+1) then return .true
      self~setRowColumn(row,column,.true)
    end
  end
  return .false

::method rowColumnFree private
  expose freeMaxs freeRows freeMins
  use strict arg row,column
  return freeRows[row+1] & freeMaxs[column+row+1] & -
    freeMins[column-row+8]

::method setRowColumn private
  expose freeMaxs freeRows freeMins
  use strict arg row,column,value
  freeRows[row+1]=value
  freeMaxs[column+row+1]=value
  freeMins[column-row+8]=value
