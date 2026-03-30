options levelb

namespace rxfnsb expose parseExec

/*============================================================================= */
/* ----------------------------------------------------------------------
 * parse_exec_plan
 *
 * Execute compiled parse plan.
 *
 * plan[] layout:
 *   base = (v-1) * 5
 *   plan[base+1] = startKind
 *   plan[base+2] = startText
 *   plan[base+3] = varName
 *   plan[base+4] = endKind
 *   plan[base+5] = endText
 *
 * plan[0] = number of variables
 * ---------------------------------------------------------------------- */
parseexec: procedure = .string[]
  arg src=.string, splan=.string,template=.string,debug=0

  rs = .string[]
  src_len = length(src)
  cursor = 1
  value=.string   /* define outside do loop */

  plan=decode_plan(splan)
  if debug>0 then do
     call log "PARSE STRING  ='"src"'"
     call log "                1---5----0----5----0----5----0----5----0----5----0"
     call log "PARSE-TEMPLATE='"template"'"
  end
  if debug=9 then do
      call log "PLAN STRING='"splan"'"
     call dump_parse_plan plan
  end

  plan_count=.int
  plan_count =plan[0] / 5
  do v = 1 to plan_count
     base = (v - 1) * 5

     startKind = plan[base + 1]
     startText = plan[base + 2]
     varName   = plan[base + 3]
     endKind   = plan[base + 4]
     endText   = plan[base + 5]

     if debug=9 then call log "PLAN-VAR["v"] START=("startKind","startText") VAR="varName " END=("endKind","endText") CURSOR="cursor

     field_start = resolve_plan_start(src, src_len, cursor, startKind, startText, v)
     if debug=9 then call log "PLAN-VAR["v"] resolved start="field_start

     if field_start < 1 then do
        rs[v] = ""
        iterate
     end

     /* no end control -> remainder */
     if endKind = 0 then do
        value = substr(src, field_start)
        if debug=9 then call log ">PLAN ASSIGN 1 "varName"='"value"' FROM="field_start" MODE=REST ENTRY="v

        rs[v] = value
        cursor = src_len + 1
        iterate
     end

     /* numeric end control */
     if endKind = 3 | endKind = 4 | endKind = 5 then do
        next_cursor = resolve_plan_end(src_len, cursor, field_start, endKind, endText)
        if debug=9 then call log "PLAN-VAR["v"] numeric end cursor="next_cursor

        if next_cursor <= field_start then do
           value = take_raw_word(src, field_start, src_len, cursor)
           cursor = field_start
           if debug=9 then call log ">PLAN ASSIGN 2 "varName"='"value"' FROM="field_start" MODE=RAWWORD ENTRY="v
        end
        else do
           value = substr(src, field_start, next_cursor - field_start)
           cursor = next_cursor
           if debug=9 then call log ">PLAN ASSIGN 3 "varName"='"value"' FROM="field_start" TO="next_cursor-1" MODE=SPAN ENTRY="v
        end
        rs[v] = value
        iterate
     end

     /* literal end control */
     if endKind = 2 then do
        p = pos(endText, src, field_start)
        if p = 0 then do
           value = substr(src, field_start)
           cursor = src_len + 1
           if debug=9 then call log ">PLAN ASSIGN 4 "varName"='"value"' FROM="field_start" MODE=LIT-REST ENTRY="v
        end
        else do
           if p <= field_start then value = ""
           else value = substr(src, field_start, p - field_start)

           cursor = p + length(endText)
           if debug=9 then call log ">PLAN ASSIGN 5 "varName"='"value"' FROM="field_start" TO="p-1" MODE=LIT ENTRY="v
        end
        rs[v] = value
        iterate
     end

     /* implicit end control */
    if endKind = 6 then do
       p1 = field_start

       /* skip leading blanks */
       do while p1 <= src_len
          ch = substr(src, p1, 1)
          if ch \= ' ' then leave
          p1 = p1 + 1
       end

       if p1 > src_len then do
          value = ""
          cursor = src_len + 1
       end
       else do
          p2 = p1
          do while p2 <= src_len
             ch = substr(src, p2, 1)
             if ch = ' ' then leave
             p2 = p2 + 1
          end

          value = substr(src, p1, p2 - p1)

          cursor = p2
          do while cursor <= src_len
             ch = substr(src, cursor, 1)
             if ch \= ' ' then leave
             cursor = cursor + 1
          end
       end
       if debug=9 then call log ">PLAN ASSIGN 6 "varName"='"value"' FROM="field_start" MODE=IMPLICIT ENTRY="v
       if varName \= '.' then rs[v] = value
       iterate
    end

     say "parse_exec_plan error: invalid end kind" endKind "for variable" varName
     return rs
  end
return rs

/* --------------------------------------------------------------
 * rebuild plan[] from flat string
 * --------------------------------------------------------------
 */
decode_plan: procedure = .string[]
  arg planStr=.string

  plan = .string[]
  posn = 1
  v = 0

  do while posn <= length(planStr)

     if substr(planStr, posn, 1) = ";" then do
        posn = posn + 1
        iterate
     end

     v = v + 1
     base = (v - 1) * 5
     plan[base + 1]=read_number(planStr, posn)
     call expect_char   planStr, posn, ","
     plan[base + 2]=read_lp_field(planStr, posn)
     call expect_char   planStr, posn, ","
     plan[base + 3]=read_lp_field(planStr, posn)
     call expect_char   planStr, posn, ","
     plan[base + 4]=read_number(planStr, posn)
     call expect_char   planStr, posn, ","
     plan[base + 5] =read_lp_field(planStr, posn)
     call expect_char   planStr, posn, ";"
  end
return plan
/* --------------------------------------------------------------
 * retrieve number from string
 * --------------------------------------------------------------
 */
read_number: procedure=.string
  arg s=.string, expose posn=.int
  n = ""
  nlen=length(s)
  do while posn <= nlen
     ch = substr(s, posn, 1)
     if ch < "0" | ch > "9" then leave
     n = n || ch
     posn = posn + 1
  end

return n
/* --------------------------------------------------------------
 * retrieve field  from string
 * --------------------------------------------------------------
 */
read_lp_field: procedure=.string
  arg s=.string, expose posn=.int
  lenstr = ""
  nlen=length(s)
  do while posn <= nlen
     ch = substr(s, posn, 1)
     if ch = ":" then leave
     lenstr = lenstr || ch
     posn = posn + 1
  end
  posn = posn + 1   /* skip ':' */
  if lenstr = "" then fldlen = 0
  else fldlen = lenstr + 0
  if fldlen = 0 then out = ""
  else out = substr(s, posn, fldlen)
  posn = posn + fldlen
return out

expect_char: procedure
  arg s=.string, expose posn=.int, wanted=.string
  ch = substr(s, posn, 1)
  if ch \= wanted then do
   ##  call raise "syntax", "40.23", "decode_plan_string error: expected '"wanted"' at position" posn "got '"ch"'"
  end
  posn = posn + 1
return

resolve_plan_start: procedure = .int
  arg src=.string, src_len=.int, cursor=.int, kind=.int, text=.string, v=.int
  p = .int
  if kind = 0 then return cursor
  if kind = 3 then do
     p = text
     if p < 1 then return 1
     if p > src_len + 1 then return src_len + 1
     return p
  end
  if kind = 4 then do
     p = cursor + text
     if p < 1 then return 1
     if p > src_len + 1 then return src_len + 1
     return p
  end
  if kind = 5 then do
     p = cursor - text
     if p < 1 then return 1
     if p > src_len + 1 then return src_len + 1
     return p
  end
  if kind = 6 then do
     p = cursor
     do while p <= src_len
        if substr(src, p, 1) \= ' ' then leave
        p = p + 1
     end
     return p
  end

  if kind = 2 then do
     /* literal start only searches in first variable, otherwise start is current cursor */
     if v = 1 then do
        p = pos(text, src, cursor)
        if p = 0 then return -1
        return p + length(text)
     end
     return cursor
  end

return -1

resolve_plan_end: procedure = .int
  arg src_len=.int, cursor=.int, field_start=.int, kind=.int, text=.string

  p = .int

  if kind = 3 then p = text
  else if kind = 4 then p = field_start + text
  else if kind = 5 then p = field_start - text
  else return -1

  if p < 1 then p = 1
  if p > src_len + 1 then p = src_len + 1
return p

take_raw_word: procedure = .string
  arg src=.string, field_start=.int, src_len=.int, cursor=.int

  p1 = field_start
  p2 = p1

  /* scan to first nonblank, but keep leading blanks */
  do while p2 <= src_len
     if substr(src, p2, 1) \= ' ' then leave
     p2 = p2 + 1
  end

  if p2 > src_len then do
     cursor = src_len + 1
     return substr(src, p1)
  end

  /* scan word */
  p3 = p2
  do while p3 <= src_len
     if substr(src, p3, 1) = ' ' then leave
     p3 = p3 + 1
  end

  /* also include trailing blanks */
  p4 = p3
  do while p4 <= src_len
     if substr(src, p4, 1) \= ' ' then leave
     p4 = p4 + 1
  end

  cursor = p4
return substr(src, p1, p4 - p1)

dump_parse_plan: procedure
  arg plan=.string[]
  plan_count =plan[0] / 5
  do v = 1 to plan_count
     base = (v - 1) * 5
     call log "PLAN["v"] START=("plan[base+1]","plan[base+2]") VAR="plan[base+3] " END=("plan[base+4]","plan[base+5]")"
  end
return

log: procedure
    arg logtxt = .string
    say ">DBG "logtxt
return