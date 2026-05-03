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
options levelb

namespace rxfnsb expose runMask

/* ------------------------------------------------------------------
 * runMask
 * Execute precompiled token arrays.
 * ------------------------------------------------------------------ */
runMask: procedure=.string
  arg usetoken=.int, fmt_token=.string[] ,a1=.string, a2="", a3="", a4="", a5="", a6="", a7="", a8="", a9="", ,
            a10="", a11="", a12="", a13="", a14="", a15="",a16=""
  vals=.string[]
  vals[1]  = a1
  vals[2]  = a2
  vals[3]  = a3
  vals[4]  = a4
  vals[5]  = a5
  vals[6]  = a6
  vals[7]  = a7
  vals[8]  = a8
  vals[9]  = a9
  vals[10] = a10
  vals[11]  = a11
  vals[12]  = a12
  vals[13]  = a13
  vals[14]  = a14
  vals[15]  = a15
  vals[16]  = a16
  out  = ""
  argi = 1

  tokenAddr = 2
  nextIdx=.int
  do forever
     if fmt_token[tokenAddr] \= "$FMTOKEN" then do
        say "ERROR: invalid header at" tokenAddr
        leave
     end
     tokenId = fmt_token[tokenAddr + 1]
     nextIdx = fmt_token[tokenAddr + 2]
     if tokenId = useToken then leave
     tokenAddr = nextIdx
  end

  do i=tokenAddr+3 to nextidx by 8  /* 3. entry in Token list contains the details */
     if fmt_token[i] = "LIT" then do
        out = out||fmt_token[i+1]
        iterate
     end

     if fmt_token[i] = "TXT" then do
        out = out || left(vals[argi], fmt_token[i+2])
        argi = argi + 1
        iterate
     end

     if fmt_token[i] = "NUM" then do
        out = out || fmtNumMaskInfo(vals[argi],fmt_token[i+2],fmt_token[i+3],fmt_token[i+4],fmt_token[i+6],fmt_token[i+5])
        argi = argi + 1
        iterate
     end
  end
return out

/* ------------------------------------------------------------------
 * Format numeric field from precomputed metadata only.
 * ------------------------------------------------------------------ */
fmtNumMaskInfo: procedure=.string
   arg value=.string, seglen=.int, intslots=.int, decslots=.int, curr=.string, hasdot=.int

   width = seglen
   if hasdot = 0 then txt = strip(trunc(value))
   else txt = strip(format(value, intslots, decslots))
   slen=.int; assembler strlen slen,txt
   if slen > width then return copies("*", width)
   txt = right(txt, width)

   if curr \= "" then do
      zero=0
      numstart = .int
      assembler FNDNBLNK numstart,txt,zero
      if numstart >= 0 then do
         numstart = numstart + 1
         if numstart > 1 then txt = overlay(curr, txt, numstart - 1, 1)
      end
   end
return txt