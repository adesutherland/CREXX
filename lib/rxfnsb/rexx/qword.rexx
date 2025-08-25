/* rexx */
options levelb

namespace rxfnsb expose qword

qword: procedure=.string
  arg line=.string, wanted=.int     /* line = input string, wanted = nth word */
  line = strip(line)                /* remove leading/trailing blanks */
  len  = length(line)               /* total length */
  if wanted < 1 then return ''      /* words start at 1, bail if invalid */

  wordno = 0                        /* count of words seen so far */
  inQuote = 0                       /* 0 = normal, 1 = currently inside quotes */
  qch = ''                          /* which quote character started it (' or ") */
  buf = ''                          /* accumulator for the current word */
  i = 1                             /* position pointer */

  do while i <= len
     ch = substr(line, i, 1)        /* current character */
  /* We are currently inside quote handling */
     if inQuote then do              /* --- handling when inside a quoted string --- */
        if ch = qch then do         /* found same quote char */
           if i < len & substr(line,i+1,1) = qch then do /* doubled quote ("") or ('') -> literal quote inside string */
              buf = buf || qch
              i = i + 2             /* skip both quotes */
           end
           else do                  /* closing quote */
              inQuote = 0           /* reset flag   */
              qch = ''
              i = i + 1
           end
        end
        else do                     /* just another character inside quoted string */
           buf = buf || ch
           i = i + 1
        end
        iterate                    /* just for clarity */
     end
  /* We are in normal string processing */
     else do                       /* --- handling when NOT inside quotes --- */
        if ch = '"' | ch = "'" then do  /* entering a quoted string */
           inQuote = 1
           qch = ch
           i = i + 1
           iterate                  /* go back to top of loop */
        end
        if ch = ' ' | ch = '09'x then do /* found a blank or a tab delimiter */
           if buf \= '' then do     /* end of a word */
              wordno = wordno + 1
              if wordno = wanted then return buf
              buf = ''
           end
           /* skip over consecutive blanks/tabs */
           do while i <= len & (substr(line,i,1) = ' ' | substr(line,i,1) = '09'x)
              i = i + 1
           end
           iterate
        end
        buf = buf || ch      /* normal (non-blank, non-quote) character of a word */
        i = i + 1
     end
  end
/* end of line reached, may still have last word in buf */
  if buf \= '' then do
     wordno = wordno + 1
     if wordno = wanted then return buf
  end
return ''   /* wanted word not found */