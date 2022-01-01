/* rexx */
options levelb
/* abbrev(string,abbrebiated-string,min-char-match) */
compare: procedure = .string
  arg string = .string, astr = .string, pad = " "
  slen=0
  alen=0
  char1=0
  char2=0
  assembler strlen slen,string
  assembler strlen alen,astr
  clen=slen
  if slen>alen then do
     astr=substr(astr,1,slen,pad)
     clen=slen
  end
  else do
     if alen>slen then string=substr(string,1,alen,pad)
     clen=alen
 end

  do i=0 to clen-1
     assembler strchar char1,string,i
     assembler strchar char2,astr,i
     if char1\=char2 then return i+1
  end

return 0

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ''

