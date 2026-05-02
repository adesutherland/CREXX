/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

 /* rexx */
  options levelb numeric_classic

  namespace rxfnsb expose time
  import _rxsysb

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
    elp = 0.0                /* Note: elp is a float ... */
    elp = _elapsed(todayms)  /*       So elp stays a float ... */
    return elp/1000
 end
 if option='R' | option='r' then return _elapsed(0)
 if option='C' | option='c' then do
    if hh>12 then xm='pm'
    else xm='am'
    return right(hh,2,'0')':'right(mm,2,'0')||xm
 end

return "time invalid option"



