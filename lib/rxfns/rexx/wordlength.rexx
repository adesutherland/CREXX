/* rexx space adds n padding chars between words */
options levelb
wordlength: procedure = .string
  arg expose string = .string, wordnum = .int
wlen=0
wordstr=word(string,wordnum)
if wrdstr="" then return 0
assembler strlen wlen,wordstr
return wlen

/* function prototype */
word: procedure = .int
arg string1 = .string, wordnum = .int


