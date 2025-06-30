/* rexx */
options levelb

namespace rxfnsb expose wordindex

wordindex: procedure=.int
   arg string=.string, wordnum=.int
   len = length(string)
   offset = 0
   count = 0
   if wordnum<1 then return 0
   if len<1     then return 0

   do while offset < len
      assembler fndnblnk offset,string,offset     /* Find the next non-blank character */
      if offset < 0 then return 0                 /* No more words, negative value to distinguish from offset=0 which is valid */
      count = count + 1
      wordStart = offset
      assembler fndblnk offset,string,offset     /* Find the next blank after the word */
      if count = wordnum then return wordstart+1
      if offset < 0 then return 0                /* No more words, negative value to distinguish from offset=0 which is valid */
   end
return 0  /* wordnum not found */