/* rexx wordpos searches for string and returns word position */
options levelb
wordpos: procedure = .string
  arg expose search = .string, string = .string, start = 1

fpos=pos(search,string,start)
wnum=words(string)
w1=word(search,1)
owp=wordindex(string,1)
do i=2 to wnum
   wp=wordindex(string,i)
   if wp>fpos then return owp
   owp=wp
end
return owp

/* function prototype */
words: procedure = .int
arg string1 = .string

word: procedure = .int
arg string1 = .string, wordnum = .int

wordindex: procedure = .int
arg string1 = .string, wordnum = .int

pos: procedure = .int
arg string1 = .string, string2 = .string, start = .int