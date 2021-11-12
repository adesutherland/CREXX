/* rexx wordpos searches for string and returns word position */
options levelb
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

i=0
do until i>=wnum
   i=i+1
   wp=wordindex(string,i)
   if wp<start then iterate
   if snum=1 then do
      if search = word(string,i) then return i
   end
   else if pos(search,string,wp) = wp then return i
end
return 0

/* function prototype */
words: procedure = .int
arg string1 = .string

word: procedure = .string
arg string1 = .string, wordnum = .int

wordindex: procedure = .int
arg string1 = .string, wordnum = .int

pos: procedure = .int
arg string1 = .string, string2 = .string, start = 1

strip: procedure = .string
       arg instr = .string, option = "B", schar= " "
