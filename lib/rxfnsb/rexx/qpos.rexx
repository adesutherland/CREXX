/* rexx */
options levelb
namespace rxfnsb expose qpos

/* ------------------------------------------------------------------
 *  QPOS(needle, text [, start])
 *  Position of needle outside quotes.
 *  Ignores needle inside '...' or "...".
 *  Quote semantics:
 *    '...' and "..." are pure delimiters.
 *    "" and '' denote an empty quoted string.
 *    No quote escaping or doubling is supported.
 *  Returns 0 if not found.
 *  ------------------------------------------------------------------
 */
qpos: procedure=.int
  arg needle=.string, text=.string, start=1

  /* quick outs */
  if start < 1 then start = 1
  if needle = "" then return 0

  lenNeedle = 0
  lenText   = 0
  assembler strlen lenNeedle,needle
  assembler strlen lenText,text
  if start > lenText then return 0

  ppi = start
  assembler strpos ppi, needle, text   /* first needle hit, may be inside quotes */
  if ppi = 0 then return 0

  q1 = '"'
  q2 = "'"
  n = start
  m = start
  assembler strpos n, q1, text
  assembler strpos m, q2, text

  /* no quotes at all → safe to return the first hit */
  if n + m = 0 then return ppi

  /* first quote position */
  if n = 0 then minq = m
  else if m = 0 then minq = n
  else if n < m then minq = n
  else minq = m

  /* hit before first quote → safe */
  if ppi < minq then return ppi

  /* continue scanning from the first quote onward */
  i = minq
  qlen = 1
  ch = ''

  do while i <= lenText
     j = i - 1                           ## setstrpos needs 0-based offset
     assembler SETSTRPOS text, j
     assembler substring ch, text, qlen  ## fetch one byte and test if quote char

     /* if we are at a quote, skip quoted part */
     if ch = "'" then do                 ## we are inside a quoted part
        n = i + 1                        ## set behind current quote, to find ending quote
        assembler strpos n, ch, text     ## find the end of the quoted part, strpos uses 1-based offset
        if n = 0 then return 0           ## no ending quote, treat as end of string, therefore no match
        i = n + 1                        ## end of quote found, move to next byte, then we are outside the quoted part
        iterate                          ## for research iterate
     end
     else if ch = '"' then do            ## we are inside a quoted part
        n = i + 1                        ## set behind current quote, to find ending quote
        assembler strpos n, ch, text     ## find the end of the quoted part, strpos uses 1-based offset
        if n = 0 then return 0           ## no ending quote, treat as end of string, therefore no match
        i = n + 1                        ## end of quote found, move to next byte, then we are outside the quoted part
        iterate                          ## for research iterate
     end
     /* try to match outside quotes */
     if ppi < i then do                  ## ensure ppi is at/after i, if not we moved i past a quoted string
        ppi = i                          ## ppi is outdated, find new needle at/after new position
        assembler strpos ppi, needle, text
        if ppi = 0 then return 0         ## no new position found, return with not found
     end
     if ppi = i then return i            ## needle must be at position ppi
     i = i + 1
  end
  return 0