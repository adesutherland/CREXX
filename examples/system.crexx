options levelb
import rxfnsb
import system
namespace main  expose myexpose

  failures = .int
  out = .string[]
  err = .string[]
  cmd_out = .string[]
  path_out = .string[]

  failures = 0
  say "Operation System:  "opsys()
  say "Current Directory: "getdir()
 /* address command "ls -l --time-style=long-iso" output out error err */
 address command "ls -l --time-style=long-iso" output out error err
  say myexpose()
  say 'exposed from call '_fred
  say _mary
return
  do i=1 to out[0]
     call LsLineToIspf out[i]
     say 12345 _fred
  end
  say "Errors"
  do i=1 to err[0]
     say err[i]
  end
return

/* Convert one "ls -l" output line to ISPF-like directory row */

LsLineToIspf: procedure=.void
  arg line=.string
  line = strip(line)

  if line = "" then return
  if word(line, 1) = "total" then return
  say line
  parse var line perms links owner group size ymd hm name

  /* Ignore malformed lines */
  if perms = "" | size = "" | name = "" then return

  type = substr(perms, 1, 1)

  select
    when type = "d" then filetype = "DIR"
    when type = "l" then filetype = "LINK"
    when type = "-" then filetype = "FILE"
    otherwise filetype = "?"
  end
  parse var ymd year '-' month '-' day
  parse var hm  hour ':' minute
  date = right(day, 2)"-"month"-"year
  fsay "{name:<32} {filetype:<6} {size:>12} {date:<12} {hm:<8} {owner:<8} {group:<8}"
  return
/*
  return left(name, 32)||" ",
    left(filetype, 6) || " ",
    right(size, 12) || "  " || ,
    left(date, 12) || " ",
    left(hm, 8) || " ",
    left(owner, 8) ||" " ,
    left(group, 8) || " ",
    perms
*/
myexpose: procedure=.int expose _fred _mary
  _fred=.string
  _fred='exposed value'
  _mary=111
return 99