/* rexx wordpos searches for string and returns word position */
options levelb

namespace rxfnsb expose wordpos

wordpos: procedure = .int
  arg expose search = .string, string = .string, start = 1

tlen=0
assembler strlen tlen,string
if tlen=0 then return 0
assembler strlen tlen,search
if tlen=0 then return 0

wnum=words(string)
if wnum=0 then return 0
snum=words(search)
if snum=0 then return 0

search=strip(search)

startpos=wordindex(string,start)  /* calculate wordpos */
do i=start to wnum
   if snum=1 then do
      if abbrev(word(string,i),search)>0 then return i
   end
   else do
      if pos(search,string,startpos) = startpos then return i
      startpos=wordindex(string,i+1)
      if startpos=0 then leave
   end
end
return 0
