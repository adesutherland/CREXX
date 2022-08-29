/* rexx */
options levelb

namespace rxfnsb
import _rxsysb

date: Procedure = .string
 arg oFormat = "", idate = "", iFormat = "", osep="", isep=""
  if iformat="" then iformat="NORMAL"
  if oformat="" then oformat="NORMAL"
  if isep\="" then isep=substr(isep,1,1)
  if osep\="" then osep=substr(osep,1,1)
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
  else if fabbreV('JDN',iformat,3)  then nop
  else if fabbreV('BASE',iformat,1) then iNorm=idate+1721426
  else if fabbreV('UNIX',iformat,2) then iNorm=idate+1721426+719162
  else if fabbreV('JULIAN',iformat,1) then do
     idate=right(idate,7,'0')
     YY=substr(idate,1,4)
     daysofyear=substr(idate,5,3)
     iNorm=_jdn(1,1,YY)+daysofyear-1
  end
  else do
     iNorm=_datei(idate,iformat,isep)
     if iNorm='SYNTAX' then return ''
  end
/* 2. Translate JDN according to its output format */

return _dateo(iNorm,oFormat,osep)

fabbreV: Procedure = .int
  arg p0 = .string, p1 = .string, flen = 1
  if substr(p0,1,flen)=substr(p1,1,flen) then return 1
return 0
