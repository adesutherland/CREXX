/* rexx */
options levelb

namespace _rxsysb
import rxfnsb

_dateo: Procedure = .string
  arg jdn = .int, format = .string, osep=""
  if format='' then format='NORMAL'

  if fabbreV('BASE',format,1)>0 then return JDN-1721426          /* BASE is REXX Type Format, starting 01.01.0001     */
  if fabbreV('UNIX',format,2)>0 then return JDN-1721426-719162   /* UNIX Days since 1.1.1970                          */
  if fabbreV('JDN',format,3)>0 then return JDN                   /* JDN is Julian Day Number starting 24. Nov 4714 BC */

  weekday="Monday Tuesday Wednesday Thursday Friday Saturday Sunday"
  mlist='January February March April May June July August September October November December'

/* Translate Julian Day Number in Gregorian Date */
  L=JDN+68569
  N=4*L%146097
  L=L-(146097*N+3)%4
  i=4000*(L+1)%1461001
  L=L-1461*i%4+31
  J=80*L%2447
  dd=L-2447*J%80
  L=J%11
  mm=J+2-12*L
  YY=100*(N-49)+I+L
  wday=(jdn//7)+1

/* YEAR= yy ; Month=mm ; Day=dd */
  if fabbreV('JULIAN',format,1) then do
     daysofyear=JDN-_jdn(1,1,YY)+1
     return right(YY,4,'0')right(daysofyear,3,'0')
  end
  if fabbreV('DAYS',format,1) then do
     daysofyear=JDN-_jdn(1,1,YY)+1
     return right(daysofyear,3,'0')
  end
  if fabbreV('WEEKDAY',format,1) then return word(weekday,wday)

  if fabbreV('CENTURY',format,1) then do
     dayscentury=jdn-_jdn(1,1,YY%100*100)+1
     return dayscentury
  end

  if fabbreV('EPOCH',format,2) then do
     return (jdn-_jdn(1,1,1970))*86400
    end

  if fabbreV('NORMAL',format,1) then return right(dd,2,'0')' 'substr(word(mlist,mm),1,3)' 'right(YY,4,'0')
  if fabbreV('XNORMAL',format,2) then return right(dd,2,'0')' 'word(mlist,mm)' 'right(YY,4,'0')  /* extended Normal */

  if fabbreV('MONTH',format,1) then return word(mlist,mm)
  if osep="" then tsep="/"
     else tsep=osep
  if fabbreV('EUROPEAN',format,1)   then return right(dd,2,'0')||tsep||right(mm,2,'0')||tsep||right(YY,2,'0')
  if fabbreV('XEUROPEAN',format,2)  then return right(dd,2,'0')||tsep||right(mm,2,'0')||tsep||right(YY,4,'0')
  if fabbreV('USA',format,1)        then return right(mm,2,'0')||tsep||right(dd,2,'0')||tsep||right(YY,4,'0')
  if fabbreV('XUSA',format,2)       then return right(mm,2,'0')||tsep||right(dd,2,'0')||tsep||right(YY,4,'0')
  if fabbreV('ORDERED',format,1)    then  return right(YY,2,'0')||tsep||right(mm,2,'0')||tsep||right(dd,2,'0')
  if fabbreV('XORDERED',format,2)   then  return right(YY,4,'0')||tsep||right(mm,2,'0')||tsep||right(dd,2,'0')
  if osep="" then tsep="."
     else tsep=osep
  if fabbreV('GERMAN',format,1)     then return right(dd,2,'0')||tsep||right(mm,2,'0')||tsep||right(YY,2,'0')
  if fabbreV('XGERMAN',format,2)    then return right(dd,2,'0')||tsep||right(mm,2,'0')||tsep||right(YY,4,'0')

  if fabbreV('STANDARD',format,2)   then return right(YY,4,'0')right(mm,2,'0')right(dd,2,'0')

  if osep="" then tsep=""
     else tsep=osep
   if fabbreV('SORTED',format,1)     then return right(YY,4,'0')||tsep||right(mm,2,'0')||tsep||right(dd,2,'0')
  if osep="" then tsep="-"
     else tsep=osep
  if fabbreV('DEC',format,3)        then  return right(dd,2,'0')||tsep||right(mm,2,'0')||tsep||right(yy,2,'0')
  if fabbreV('XDEC',format,3)       then  return right(dd,2,'0')||tsep||right(mm,2,'0')||tsep||right(yy,4,'0')
  if fabbreV('INTERNATIONAL',format,3) then  return right(YY,4,'0')||tsep||right(mm,2,'0')||tsep||right(dd,2,'0')

  if fabbreV('QUALIFIED',format,1) then return word(weekday,wday)', 'word(mlist,mm)' 'right(dd,2,'0')', 'right(YY,4,'0') /* Thursday, December 17, 2020 */

return right(dd,2,'0')' 'right(mm,2,'0')' 'right(YY,4,'0')

fabbreV: Procedure = .int
  arg p0 = .string, p1 = .string, flen = 1
  if substr(p0,1,flen)=substr(p1,1,flen) then return 1
return 0
