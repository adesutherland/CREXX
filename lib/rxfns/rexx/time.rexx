 /* rexx */
  options levelb

time: procedure = .string
   arg option = ""

 if option="" then option='N'
 option=upper(option)
 todayms=0
 tstring=""

 if option='ZD'  then do
    assembler xtime todayms,"Z"
    return todayms
 end
 if option='T'  then do
    assembler xtime todayms,"T"
    return todayms
  end
  if option='TS'  then do
     assembler xtime todayms,"C"
     return todayms
 end
 if option='ZN'  then do
    tstring="ABC"
    assembler xtime tstring,"N"
    return tstring
 end

 if option="UTC" then do
    assembler xtime todayms,"U"
    option='N'
 end
 else assembler mtime todayms

 ms=todayms//1000000
 today=todayms%1000000
 hh=today%3600
 rm=today//3600
 mm=rm%60
 ss=rm//60

 if option='N' then return right(hh,2,'0')':'right(mm,2,'0')':'right(ss,2,'0')
 if option='L' then return right(hh,2,'0')':'right(mm,2,'0')':'right(ss,2,'0')'.'left(ms,6,'0')
 if option='H' then return hh
 if option='M' then return hh*60+mm
 if option='S' then return today
 if option='US' then return todayms
 if option='E' then do
    elp=_elapsed(todayms)
    return elp/1000.1   /* as we don't have ms, we fake it, by a weird divide with 1000.1 to create a float */
 end
 if option='R' | option='r' then return _elapsed(0)
 if option='C' | option='c' then do
    if hh>12 then xm='pm'
    else xm='am'
    return right(hh,2,'0')':'right(mm,2,'0')||xm
 end

return "time invalid option"

/* Prototype functions */
trunc: procedure = .string
  arg number = .float, fraction = 0
right: procedure = .string
       arg string = .string, length1 = .int, pad = '0'
left: procedure = .string
       arg string = .string, length1 = .int, pad = '0'
_elapsed: procedure = .int
       arg string1 = .int
upper: procedure = .string
       arg string1 = .string




