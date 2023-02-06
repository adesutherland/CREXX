/* rexx */
options levelb

namespace rxfnsb expose abbrev

/* abbrev(string,abbrebiated-string,min-char-match) */
abbrev: procedure = .string
  arg string = .string, astr = .string, len = 0
  slen=0
  alen=0
  char1=0
  char2=0
  assembler strlen slen,string
  assembler strlen alen,astr
  do i=0 to alen-1
     assembler strchar char1,string,i
     assembler itos char1
     assembler strchar char2,astr,i
     assembler itos char2
     if char1\=char2 then return 0
  end
  /* is the minimum match of chars satisfied */
  if len>0 then do
     if i<len then return 0
  end
return 1
