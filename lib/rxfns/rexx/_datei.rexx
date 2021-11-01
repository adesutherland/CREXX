/* rexx */
options levelb

_datei: Procedure = .string
 arg idate = .string, format = .string
/* ----------------------------------------------------------
 *  Converts given Date in Julian Day Number
 * ----------------------------------------------------------
 */
  if format='' then format='NORMAL'

  if abbreV('DAYS',format,1) then return 'DAYS not allowed as input format'
  if abbreV('CENTURY',format,1) then return 'CENTURY not allowed as input format'
  if abbreV('EUROPEAN',format,1) | abbreV('GERMAN',format,1) | abbreV('XEUROPEAN',format,1) | abbreV('XGERMAN',format,1) | abbreV('DEC',format,3) | abbreV('XDEC',format,3) then do  /* format dd.mm.yy or dd/mm/yy */
     idate=split(idate)
     YY=word(idate,3)
     yy=testyear(yy)
     mm=word(idate,2)
     dd=word(idate,1)
     return _jdn(dd,mm,YY)
  end
  if abbreV('USA',format,1)  | abbreV('XUSA',format,2) then do  /* format dd.mm.yy or mm/dd/yy */
     idate=split(idate)
     YY=word(idate,3)
     yy=testyear(yy)
     mm=word(idate,1)
     dd=word(idate,2)
     return _jdn(dd,mm,YY)
  end
  if abbreV('INTERNATIONAL',format,2)  then do  /* format yyyy-mm-dd */
       idate=split(idate)
       YY=word(idate,1)
       yy=testyear(yy)
       mm=word(idate,2)
       dd=word(idate,3)
       return _jdn(dd,mm,YY)
    end
if abbreV('QUALIFIED',format,2)  then do  /* format Thursday, December 17, 2020 */
       mlist='JANUARY FEBRUARY MARCH APRIL MAY JUNE JULY AUGUST SEPTEMBER OCTOBER NOVEMBER DECEMBER'
       idate=split(idate)
       YY=word(idate,4)
       yy=testyear(yy)
       mm=word(idate,2)
       dd=word(idate,3)
       dd=split(dd)
       mm=wordpos(mm,mlist)
       return _jdn(dd,mm,YY)
    end

  if abbreV('STANDARD',format,1) then do
     idate=right(idate,8,'0')
     YY=substr(idate,1,4)
     yy=testyear(yy)
     mm=substr(idate,5,2)
     dd=substr(idate,7,2)
     return _jdn(dd,mm,YY)
  end
  if abbreV('ORDERED',format,1) then do
     idate=right(idate,10,'0')
     YY=substr(idate,1,4)
     mm=substr(idate,6,2)
     dd=substr(idate,9,2)
     return _jdn(dd,mm,YY)
  end
  if abbreV('EPOCH',format,3) then do
     idate=idate%86400+_jdn(1,1,1970)
     return idate
  end
  if abbreV('JDN',format,3) then return idate                   /* is already JDN */
  if abbreV('BASE',format,1) then return idateJ+1721426         /* calulate JDN   */
  if abbreV('UNIX',format,2)>0 then return idate+1721426+719162 /* UNIX Days since 1.1.1970 */

return format' invalid input format'

testyear: procedure = .int
  arg year = .int
  if year%1000=0 then return 2000+year
return year

split: Procedure = .string
  arg idate = .string
  xlen=0
  retstr=""
  c0=" "
  assembler strlen xlen,idate

  do i=1 to xlen
      c0=substr(idate,i,1)
      if c0="." | c0="/" | c0="," | c0="-" then retstr=retstr||" "
      else retstr=retstr||c0
  end
return retstr


/* Prototypes */

_jdn: Procedure = .int
  arg day = .int, month = .int, year = .int

abbrev: procedure = .string
  arg string = .string, astr = .string, len = 0

right:  procedure = .string
  arg string = .string, len =  .int, pad = " "

word: procedure = .string
  arg string1 = .string, string2 = .int

wordpos: procedure = .int
  arg string1 = .string, string2 = .string, offset = 0


length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ''



