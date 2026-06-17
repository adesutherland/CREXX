options levelb
import rxfnsb
command = .string[]
command[1] = '-I'
command[2] = 'AAA'
command[3] = '-O'
command[4] = 'BBB'
command[5] = '-M'
command[6] = 'CCC'

infile = ''
outfile = ''
maclib = ''

do i = 1 to command[0]
  j = i + 1
  command[i] = upper(command[i])
  if command[i] = '-I' then infile = command[j]
  else if command[i] = '-IN' then infile = command[j]
  else if command[i] = '-O' then outfile = command[j]
  else if command[i] = '-OUT' then outfile = command[j]
  else if command[i] = '-M' then maclib = command[j]
  else if command[i] = '-VERBOSE0' then nop
end

say 'infile=' || infile
say 'outfile=' || outfile
say 'maclib=' || maclib
return
