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

namespace rxfnsb expose delword

/* delword(string,wordnumber-to-delete,length) delete one word, or the remaining words in string */
delword: procedure = .string
  arg string = .string, wnum = .int, wcount = -1

  if wnum<1    then return string
  if wcount=0  then return string

  wrds=words(string)
  if wnum>wrds then return string

  wdel=0
  wlen=0
  slen=0
  wpos=0
  retstr = ""

  if wcount<0 then do
     wpos=wordindex(string,wnum)    /* locate position of word x */
     if wpos=0 then return string
     if wpos=1 then return ""
     return substr(string,1,wpos-1)
  end

  do forever
     assembler strlen slen,string   /* length of original string */
     if slen<1 then return ""
     wpos=wordindex(string,wnum)    /* locate position of word x */
     if wpos=0 then return string
     wrd= word(string,wnum)         /* load word      */
     assembler strlen wlen,wrd      /* length of word */
     xpos=wpos+wlen                 /* next position following word */
     if xpos<slen then do           /* if still in source string, check if char is blank */
        if substr(string,xpos,1)=' ' then wlen=wlen+1 /* if so, take it as part of word, increase word length */
     end
     if wpos+wlen>slen then do      /* if next byte after word exceeds length, it's last word */
        if wpos=1 then retstr=''    /* if start position 1: empty string remains              */
        else retstr=substr(string,1,wpos-1)  /* else take string prior to word                */
     end
     else do                        /* next word is within string        */
        if wpos=1 then retstr=substr(string,wpos+wlen)   /* take remain string after word     */
        else do                     /* drop word and re-construct string */
           if wnum<wrds then retstr=substr(string,1,wpos-1)||substr(string,wpos+wlen)  /* drop was in the middle */
           else retstr=substr(string,1,wpos-1)  /* drop was last word  */
        end
     end
     string=retstr         /* move back to original string and loop    */
     wdel=wdel+1           /* increase deleted count                   */
     if wdel>=wcount then return string /* match it with the requested count? Then return */
  end
return string
