/* rexx qwordpos searches for string and returns word position */
options levelb

namespace rxfnsb expose qwordpos

qwordpos: procedure = .int
  arg expose search = .string, string = .string, start = 1

  tlen=0
  assembler strlen tlen,string
  if tlen=0 then return 0
  assembler strlen tlen,search
  if tlen=0 then return 0

  wnum=qwords(string)
  if wnum=0 then return 0
  snum=qwords(search)
  if snum=0 then return 0

  search=strip(search)

  startpos=qwordindex(string,start)  /* calculate wordpos */
  do i=start to wnum
     ppi=pos(search,qword(string,i))
     if ppi=1 | ppi=2 then return i
  end
return 0
