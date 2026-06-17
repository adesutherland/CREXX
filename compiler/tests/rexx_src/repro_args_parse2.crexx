options levelb
import rxfnsb
arr = .string[]
arr.1 = '-I'
arr.2 = 'AAA'
arr.3 = '-O'
arr.4 = 'BBB'
arr.5 = '-M'
arr.6 = 'CCC'

infile = ''
outfile = ''
maclib = ''

do i = 1 to 5 by 2
  j = i + 1
  arr.i = upper(arr.i)
  if arr.i = '-I' then infile = arr.j
  else if arr.i = '-IN' then infile = arr.j
  else if arr.i = '-O' then outfile = arr.j
  else if arr.i = '-OUT' then outfile = arr.j
  else if arr.i = '-M' then maclib = arr.j
end

say 'infile=' || infile
say 'outfile=' || outfile
say 'maclib=' || maclib
return
