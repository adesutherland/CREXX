/* rexx */
options levelb

namespace rxfnsb expose qwordindex

qwordindex: procedure=.string
  arg line=.string, wanted=.int
  len = length(line)
  if wanted < 1 then return 0
  wordno = 0
  inQuote= 0
  startpos = 0
  qch=''
  buf    = ''
  i = 1
  do while i <= len
     ch = substr(line,i,1)
     if inQuote then do
        if ch = qch then do    /* doubled quote => literal quote */
           if i < len & substr(line,i+1,1) = qch then do
              buf = buf || qch
              i = i + 2
              iterate
           end
           else do
              inQuote = 0
              qch = ''
              i = i + 1
              iterate
           end
        end
        else do
           buf = buf || ch
           i = i + 1
           iterate
        end
     end
     else do
        if ch = '"' | ch = "'" then do
          if buf = '' then startpos = i
          inQuote = 1
          qch = ch
          i = i + 1
          iterate
        end
        if ch = ' ' | ch = '09'x then do
           if buf \= '' then do
              wordno = wordno + 1
              if wordno = wanted then return startpos
              buf = ''
              startpos = 0
           end
           do while i <= len & (substr(line,i,1) = ' ' | substr(line,i,1) = '09'x)
              i = i + 1
           end
           iterate
        end
        if buf = '' then startpos = i
        buf = buf || ch
        i = i + 1
     end
  end
  if buf \= '' then do
     wordno = wordno + 1
     if wordno = wanted then return startpos
  end
return 0   /* not found */