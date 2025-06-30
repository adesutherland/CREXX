/* rexx */
options levelb

namespace rxfnsb expose word

word: procedure=.string
  arg string=.string, wordnum=.int
  len = length(string)
  offset = 0
  count = 0
  if wordnum<1 then return ""
  if len<1 then return ""

  do while offset < len
     assembler fndnblnk offset,string,offset     /* Find the next non-blank character */
     if offset < 0 then return ""   /* No more words */
     count = count + 1
     wordStart = offset
     assembler fndblnk offset,string,offset    /* Find the next blank after the word */
     if count = wordnum then do
        if offset < 0 then return substr(string, wordStart + 1)
        else return substr(string, wordStart + 1, offset - wordStart)
     end
     if offset < 0 then leave
  end
return ""  /* wordnum not found */
