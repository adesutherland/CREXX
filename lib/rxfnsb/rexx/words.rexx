/* rexx */
options levelb

namespace rxfnsb expose words

words: procedure=.int
  arg string=.string
  len = length(string)
  offset = 0
  count = 0
  if wordnum<1 then return 0
  if len<1 then return 0

do while offset < len
    /* Find next non-blank character (start of a word) */
    assembler fndnblnk offset,string,offset
    if offset < 0 then leave

    count = count + 1  /* Found a word */

    /* Find next blank after this word (to skip it) */
    assembler fndblnk offset,string,offset
    if offset < 0 then leave
  end
return count  /* wordnum not found */