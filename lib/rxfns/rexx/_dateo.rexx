/* rexx */
options levelb

_dateo: Procedure = .string
  arg jdn = .int, format = .string
  if format='' then format='NORMAL'

  if abbreV('BASE',format,1)>0 then return JDN-1721426          /* BASE is REXX Type Format, starting 01.01.0001     */
  if abbreV('UNIX',format,1)>0 then return JDN-1721426-719162   /* UNIX Days since 1.1.1970                          */
  if abbreV('JDN',format,1)>0 then return JDN                   /* JDN is Julian Day Number starting 24. Nov 4714 BC */

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

/* YEAR= yy ; Month=mm ; Day=dd */
  if abbreV('JULIAN',format,1) then do
     daysofyear=JDN-_jdn(1,1,YY)+1
     return right(YY,4,'0')right(daysofyear,3,'0')
  end
  if abbreV('DAYS',format,1) then do
     daysofyear=JDN-_jdn(1,1,YY)+1
     return right(daysofyear,3,'0')
  end
  if abbreV('WEEKDAY',format,1) then do
     jdn=jdn//7
     if jdn=0 then return 'Monday'
     if jdn=1 then return 'Tuesday'
     if jdn=2 then return 'Wednesday'
     if jdn=3 then return 'Thursday'
     if jdn=4 then return 'Friday'
     if jdn=5 then return 'Saturday'
     if jdn=6 then return 'Sunday'
  end

  if abbreV('CENTURY',format,1) then do
     dayscentury=jdn-_jdn(1,1,YY%100*100)+1
     return dayscentury
  end

  if abbreV('SHORT',format,2) then do
     list='JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC'
     ms=word(list,mm)
     return right(dd,2,'0')' 'ms' 'right(YY,4,'0')
  end
  if abbreV('LONG',format,1) then do
     list='JANUARY FEBRUARY MARCH APRIL MAY JUNE JULY AUGUST SEPTEMBER OCTOBER NOVEMBER DECEMBER'
     ms=word(list,mm)
     return right(dd,2,'0')' 'ms' 'right(YY,4,'0')
  end

  if abbreV('MONTH',format,1) then do
       list='JANUARY FEBRUARY MARCH APRIL MAY JUNE JULY AUGUST SEPTEMBER OCTOBER NOVEMBER DECEMBER'
       return word(list,mm)
    end

  if abbreV('EUROPEAN',format,1)   then return right(dd,2,'0')'/'right(mm,2,'0')'/'right(YY,2,'0')
  if abbreV('XEUROPEAN',format,2)  then return right(dd,2,'0')'/'right(mm,2,'0')'/'right(YY,4,'0')

  if abbreV('GERMAN',format,1)     then return right(dd,2,'0')'.'right(mm,2,'0')'.'right(YY,2,'0')
  if abbreV('XGERMAN',format,2)    then return right(dd,2,'0')'.'right(mm,2,'0')'.'right(YY,4,'0')

  if abbreV('USA',format,1)        then return right(mm,2,'0')'/'right(dd,2,'0')'/'right(YY,4,'0')
  if abbreV('XUSA',format,2)       then return right(mm,2,'0')'/'right(dd,2,'0')'/'right(YY,4,'0')

  if abbreV('STANDARD',format,1)   then return right(YY,4,'0')right(mm,2,'0')right(dd,2,'0')

  if abbreV('ORDERED',format,1)    then  return right(YY,2,'0')'/'right(mm,2,'0')'/'right(dd,2,'0')
  if abbreV('XORDERED',format,2)   then  return right(YY,4,'0')'/'right(mm,2,'0')'/'right(dd,2,'0')

  if abbreV('NORMAL',format,1)    then  return right(dd,2,'0')' 'right(mm,2,'0')' 'right(yy,4,'0')

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


