/* rexx */
options levelb

/* delstr(string,position,length) delete string from certain position and length and returns it */
delstr: procedure = .string
  arg expose string = .string, position = .int, dellen = .int

  if position<1 then position=1
  len=length(string)
  if position>len then return string
  if position+dellen>len then dellen=0
  if position=1 then do
     if dellen>0 then retstr=substr(string,dellen+1)
     else retstr=''
  end
  else do
    if dellen=0 then retstr=substr(string,1,position-1)
    else retstr=substr(string,1,position-1)||substr(string,position+dellen)
  end
return retstr

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '