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

  ppi=start
  assembler strpos ppi,needle,text
  if ppi=0 then return 0     /* if string is not contained, we don't need to check for quotes */
  lenNeedle = 0
  lenText   = 0
  assembler strlen lenNeedle,needle
  assembler strlen lenText,text
  q1='"'
  q2="'"
  n = start
  m = start
  assembler strpos n, q1, text
  assembler strpos m, q2, text
/* no quotes at all → safe to return the first hit */
  if n + m = 0 then return ppi
/* first quote position */
  if n = 0 then minq = m
  else if m = 0 then minq = n
  else if n<m then minq = n
  else minq=m
/* hit before first quote → safe */
  if ppi < minq then return ppi

/* continue scanning from the first quote onward */
  i = minq
  do while i<=LenText     /* scan through full string */
     equote=0
     ch = substr(text, i, 1)
  /* toggle quote state */
     if ch = "'" | ch = '"' then do
        n=i+1
        assembler strpos n,ch,text
        if n=0 then return 0
        i=n+1
        equote=1
     end
  /* match if not inside quotes */
     if substr(text, i, lenNeedle) = needle then return i
     if equote = 0 then i = i + 1    /* only advance if no quote jump happened */
     equote = 0
  end
return 0