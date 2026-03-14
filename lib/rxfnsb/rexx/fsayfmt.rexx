/* rexx */
options levelb

namespace rxfnsb expose fsayfmt

fsayFmt: Procedure=.string
  arg txt=.string
  start = 1
  tindx = 0
  out = ""
  txt=strip(txt)
  qt=substr(txt,1,1)
  if qt="'" | qt='"' then txt=substr(txt,2,length(txt)-2)
  else qt="'"
  do forever
     firstPos = qpos("{", txt, start)
     if firstPos = 0 then leave
     firstClose = qpos("}", txt, firstPos + 1)

     if firstClose = 0 then return txt
    /* literal text before placeholder */
     lit = substr(txt, start, firstPos - start)
     if lit \= "" then do
        if out \= "" then out = out || "||"
        out = out ||qt||lit||qt
     end
   /* placeholder */
     segment = substr(txt, firstPos + 1, firstClose - firstPos - 1)
     replace = fmtSegment(segment)
     if out \= "" then out = out || "||"
     out = out ||replace
     start = firstClose + 1
  end
  lit = substr(txt, start)
  if lit \= "" then do
     if out \= "" then out = out || "||"
    out = out ||qt||lit||qt
  end
return out

fmtSegment: procedure = .string
   arg segment = .string
   segment=translate(segment,,':')
   vname = word(segment,1)
   fmtspec = word(segment,2)
   align = ""
   width = ""
   decs  = ""
   if fmtspec \= "" then do
      c1 = substr(fmtspec,1,1)
      if c1='<' | c1='>' | c1='^' then do
         align = c1
         fmtspec = substr(fmtspec,2)
      end
   end
   ppi = pos('.', fmtspec)
   if ppi = 0 then width = fmtspec
   else do
      width = substr(fmtspec,1,ppi-1)
      decs  = substr(fmtspec,ppi+1)
   end
   fmt = vname
   if decs \= "" then do
      fmt = "format("vname","width","decs")"
      if align = '<' then fmt = "left("fmt","width")"
      else if align = '^' then fmt = "center("fmt","width")"
      /* '>' or none: FORMAT already gives right-justified numeric width */
      return fmt
   end
   if width \= "" then do
      if align = '<' then fmt = "left("vname","width")"
      else if align = '>' then fmt = "right("vname","width")"
      else if align = '^' then fmt = "center("vname","width")"
   end
return fmt