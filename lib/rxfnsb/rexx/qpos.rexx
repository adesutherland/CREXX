/* rexx */
options levelb

namespace rxfnsb expose qpos

/* ------------------------------------------------------------------
 *  QPOS(needle, text [, start])  position of needle outside quotes
 *
 *  Scans 'text' for 'needle', ignoring any inside '...' or "..." strings.
 *  Handles doubled quotes '' and "" properly.
 *  Returns 0 if not found.
 *  ------------------------------------------------------------------
 */
qpos: procedure=.int
  arg needle=.string, text=.string, start=1
  lenNeedle = 0
  lenText   = 0
  assembler strlen lenNeedle,needle
  assembler strlen lenText,text

  q1='"'
  q2='"'
  n=start
  m=start
  assembler strpos n,q1,text
  assembler strpos m,q2,text
  if n+m=0 then return pos(needle,text,start)
/* search for contained quotes */
  i=start
  do while i<=LenText     /* scan through full string */
     ch = substr(text, i, 1)
  /* toggle quote state */
     if ch = "'" | ch = '"' then do
        n=i+1
        assembler strpos n,ch,text
        if n=0 then return 0
        i=n+1
     end
  /* match if not inside quotes */
     if substr(text, i, lenNeedle) = needle then return i
     i=i+1
  end
return 0