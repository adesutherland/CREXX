 /* rexx */
  options levelb

time: procedure = .string
   arg option = ""

 if option="" then option='N'
 todayms=0
 assembler mtime todayms
 ms=todayms//1000000
 today=todayms%1000000
 hh=today%3600
 rm=today//3600
 mm=rm%60
 ss=rm//60

 if option='N' | option='n' then return right(hh,2,'0')':'right(mm,2,'0')':'right(ss,2,'0')
 if option='L' | option='l' then return right(hh,2,'0')':'right(mm,2,'0')':'right(ss,2,'0')'.'left(ms,6,'0')
 if option='H' | option='h' then return hh
 if option='M' | option='m' then return hh*60+mm
 if option='S' | option='s' then return today
 if option='E' | option='e' then do
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




