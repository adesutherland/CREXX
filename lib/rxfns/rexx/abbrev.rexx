/* rexx */
options levelb
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

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ''

