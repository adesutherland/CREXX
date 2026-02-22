options levelb

parseproc: procedure
  arg command = .string[]
  infile = ''
  outfile = ''
  maclib = ''
  do i = 1 to command.0
    j = i + 1
    command.i = upper(command.i)
    if command.i = '-I' then infile = command.j
    else if command.i = '-IN' then infile = command.j
    else if command.i = '-O' then outfile = command.j
    else if command.i = '-OUT' then outfile = command.j
    else if command.i = '-M' then maclib = command.j
  end
  say 'infile=' || infile
  say 'outfile=' || outfile
  say 'maclib=' || maclib
  return

/* Build an argument array */
args = .string[]
args.1 = '-I'
args.2 = 'AAA'
args.3 = '-O'
args.4 = 'BBB'
args.5 = '-M'
args.6 = 'CCC'
args.0 = 6

call parseproc args
return
