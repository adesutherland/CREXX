/* rexx */
options levelb

namespace rxfnsb expose subword

subword: procedure=.string
   arg string=.string, wordnum=.int
   if wordnum<1 then return ''
   spos=wordindex(string,wordnum)              /* start position of selected word */
   if spos=0 then return ""
return substr(string,spos)

