/* rexx */
options levelb

date: Procedure = .string
 arg oFormat = "", idate = "", iFormat = ""

  if iformat="" then iformat="NORMAL"
  if oformat="" then oformat="NORMAL"

  iformat=upper(iformat)
  oformat=upper(oformat)
  idate=upper(idate)

 /* 1. Check and translate input data according to its format to a JDN */
  if idate="" then do      /* no date set, take today, ignore input format */
     today=0
     assembler time today
     assembler itos today
     iNorm=today%86400+1721426+719162
  end
  else if abbreV('JDN',iformat,3)  then nop
  else if abbreV('BASE',iformat,1) then iNorm=idate+1721426
  else if abbreV('UNIX',iformat,1) then iNorm=idate+1721426+719162
  else if abbreV('JULIAN',iformat,1) then do
     idate=right(idate,7,'0')
     YY=substr(idate,1,4)
     daysofyear=substr(idate,5,3)
     iNorm=_jdn(1,1,YY)+daysofyear-1
  end
  else iNorm=_datei(idate,iformat)

/* 2. Translate JDN according to its output format */

return _dateo(iNorm,oFormat)

/* Prototype functions */
_jdn: Procedure = .int
  arg day = .int, month = .int, year = .int

_datei: Procedure = .string
  arg jdn = .string, format=""

_dateo: Procedure = .string
  arg jdn = .int, format = .string

abbrev: procedure = .string
  arg string = .string, astr = .string, len = 0

right:  procedure = .string
  arg string = .string, len =  .int, pad = " "

word: procedure = .string
  arg string1 = .string, string2 = .int

length: procedure = .int
  arg string1 = .string

substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ''

upper: procedure = .string
   arg string1 = .string




