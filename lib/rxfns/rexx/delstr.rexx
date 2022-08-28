/* rexx */
options levelb

namespace rxfnsb

/* delstr(string,position,length) delete string from certain position and length and returns it */
delstr: procedure = .string
  arg expose string = .string, position = .int, dellen = 0

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
