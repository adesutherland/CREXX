/* rexx */
options levelb

_dateo: Procedure = .string
  arg jdn = .int, format = .string
  if format='' then format='NORMAL'

  if abbreV('BASE',format,1)>0 then return JDN-1721426          /* BASE is REXX Type Format, starting 01.01.0001     */
  if abbreV('UNIX',format,2)>0 then return JDN-1721426-719162   /* UNIX Days since 1.1.1970                          */
  if abbreV('JDN',format,3)>0 then return JDN                   /* JDN is Julian Day Number starting 24. Nov 4714 BC */

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
  if abbreV('JULIAN',format,1) then do
     daysofyear=JDN-_jdn(1,1,YY)+1
     return right(YY,4,'0')right(daysofyear,3,'0')
  end
  if abbreV('DAYS',format,1) then do
     daysofyear=JDN-_jdn(1,1,YY)+1
     return right(daysofyear,3,'0')
  end
  if abbreV('WEEKDAY',format,1) then return word(weekday,wday)

  if abbreV('CENTURY',format,1) then do
     dayscentury=jdn-_jdn(1,1,YY%100*100)+1
     return dayscentury
  end

  if abbreV('EPOCH',format,2) then do
     return (jdn-_jdn(1,1,1970))*86400
    end

  if abbreV('NORMAL',format,2) then return right(dd,2,'0')' 'substr(word(mlist,mm),1,3)' 'right(YY,4,'0')
  if abbreV('XNORMAL',format,1) then return right(dd,2,'0')' 'word(mlist,mm)' 'right(YY,4,'0')  /* extended Normal */

  if abbreV('MONTH',format,1) then return word(mlist,mm)

  if abbreV('EUROPEAN',format,1)   then return right(dd,2,'0')'/'right(mm,2,'0')'/'right(YY,2,'0')
  if abbreV('XEUROPEAN',format,2)  then return right(dd,2,'0')'/'right(mm,2,'0')'/'right(YY,4,'0')

  if abbreV('GERMAN',format,1)     then return right(dd,2,'0')'.'right(mm,2,'0')'.'right(YY,2,'0')
  if abbreV('XGERMAN',format,2)    then return right(dd,2,'0')'.'right(mm,2,'0')'.'right(YY,4,'0')

  if abbreV('USA',format,1)        then return right(mm,2,'0')'/'right(dd,2,'0')'/'right(YY,4,'0')
  if abbreV('XUSA',format,2)       then return right(mm,2,'0')'/'right(dd,2,'0')'/'right(YY,4,'0')

  if abbreV('STANDARD',format,1)   then return right(YY,4,'0')right(mm,2,'0')right(dd,2,'0')

  if abbreV('ORDERED',format,1)    then  return right(YY,2,'0')'/'right(mm,2,'0')'/'right(dd,2,'0')
  if abbreV('XORDERED',format,2)   then  return right(YY,4,'0')'/'right(mm,2,'0')'/'right(dd,2,'0')

  if abbreV('DEC',format,3)        then  return right(dd,2,'0')'-'right(mm,2,'0')'-'right(yy,2,'0')
  if abbreV('XDEC',format,3)       then  return right(dd,2,'0')'-'right(mm,2,'0')'-'right(yy,4,'0')

  if abbreV('INTERNATIONAL',format,3) then  return right(YY,4,'0')'-'right(mm,2,'0')'-'right(dd,2,'0')

  if abbreV('QUALIFIED',format,1) then return word(weekday,wday)', 'word(mlist,mm)' 'right(dd,2,'0')', 'right(YY,4,'0') /* Thursday, December 17, 2020 */

 return right(dd,2,'0')' 'right(mm,2,'0')' 'right(YY,4,'0')

/* Prototypes */

_jdn: Procedure = .int
  arg day = .int, month = .int, year = .int

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



