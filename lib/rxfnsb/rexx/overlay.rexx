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

namespace rxfnsb expose overlay

/* overlay(insstr,string,position,length,pad) overlays string into existing string at certain position and length */
overlay: procedure = .string
  arg insstr = .string, string = .string, position = .int, len = 0, pad = ""
  padlen=0
  slen=0
  ilen=0
  assembler strlen padlen,pad      /* padding char */
  assembler strlen slen,string     /* source string  */
  if padlen=0 then pad=" " /* define default pad char, just in case we need one */

  if slen=0 then do
     string=pad
     slen=1
  end

  assembler strlen ilen,insstr
  if ilen=0 then do         /* insert string */
     if padlen=0 then return string
     insstr=pad
  end

  if len>0 then do                           /* was there an insert string length?      */
     if padlen>1 then pad=substr(pad,1,1)
     newins=substr(insstr,1,len,pad)         /* format insert string to requested length*/
     insstr=newins
   end
   assembler strlen ilen,insstr
   if position+ilen>slen then string=substr(string,1,position+ilen,pad)  /* is position>string length , extend string */

  if position=1 then str1=''                /* if insert position=1 then str1 is empty */
     else str1=substr(string,1,position-1)  /*    else split string at insert position */
  if position+ilen>slen then str2=''             /* if string was extended str2 is empty    */
     else str2=substr(string,position+ilen) /*    else create str2                     */

 return str1||insstr||str2                   /* return newly constructed string         */
