 /* rexx */
  options levelb

time: procedure = .string
   arg option = ""

 if option="" then option='N'
 today=0
 assembler mtime today
 ms=today//1000
 today=today%1000
 hh=today%3600
 rm=today//3600
 mm=rm%60
 ss=rm//60
 if option='N' | option='n' then return right(hh,2,'0')':'right(mm,2,'0')':'right(ss,2,'0')
 if option='L' | option='l' then return right(hh,2,'0')':'right(mm,2,'0')':'right(ss,2,'0')'.'left(ms,3,'0')
 if option='H' | option='h' then return hh
 if option='M' | option='m' then return hh*60+mm
 if option='S' | option='s' then return today
 if option='E' | option='e' then do
    g0=today
 end
 if option='C' | option='c' then do
    if hh>12 then xm='pm'
    else xm='am'
    return right(hh,2,'0')':'right(mm,2,'0')||xm
 end

return "invalid option"

/* Prototype functions */
right: procedure = .string
       arg string = .string, length1 = .int, pad = '0'
left: procedure = .string
       arg string = .string, length1 = .int, pad = '0'

