options levelb

namespace rxfnsb expose parseexec

/* ----------------------------------------------------------------------
 * parseexec
 *
 * Sequential PARSE executor for stream plan:
 *
 *   kind,len:text;
 *   kind,len:text;
 *   ...
 *
 * Kinds
 *   1 = variable / target
 *   2 = literal
 *   3 = absolute position
 *   4 = relative forward
 *   5 = relative backward
 *   6 = implicit next-word control
 * ---------------------------------------------------------------------- */
parseExec: procedure = .string[]
  arg src=.string, splan=.string, template=.string, debug=0

  rs = .string[]               /* result array returned to caller */
  src_len = length(src)
  cursor = 1                   /* current parse cursor in source string */
  pendingVar = ""              /* current variable waiting for a boundary */
  captureStart = 0             /* start position for current pending variable */
  planKind=.int[]              /* decoded plan item kinds */
  planText=.string[]           /* decoded plan item payloads */
  value=.string
  preCount = 0                 /* buffered pre-start control count */
  preKind  = .int[]            /* buffered pre-start control kinds */
  preText  = .string[]         /* buffered pre-start control texts */
  sharedKind = 0               /* one control shared from previous boundary */
  sharedText = ""

  plans = decode_stream_plan(splan,planKind,planText)

  if debug>0 then do
     call log "----- NEW PARSE V2 -----"
     call log "PARSE STRING  ='"src"'"
     call log "                1---5----0----5----0----5----0----5----0----5----0"
     call log "PARSE-TEMPLATE='"template"'"
  end
  if debug=9 then do
     call log "PLAN V2='"splan"'"
     call dump_stream_plan planKind,planText
  end

  do i = 1 to plans
     kind = planKind[i]
     text = planText[i]

     if debug=9 then call log "ITEM["i"] KIND=("kind","text") CURSOR="cursor" PENDING=<"pendingVar"> START="captureStart

     /* --------------------------------------------------------------
      * Variable target:
      *   - if another variable is still pending, close it implicitly
      *   - inject any shared boundary from the previous item
      *   - apply buffered pre-start controls
      *   - start a new capture
      * -------------------------------------------------------------- */
      if kind = 1 then do
         if pendingVar \= "" then do
            /* adjacent variables imply implicit-word termination */
            final_cursor = finalize_implicit(src, src_len, captureStart, pendingVar, rs, cursor, debug)
            cursor = final_cursor
            pendingVar = ""
            captureStart = 0
         end

         /* inject shared boundary/start from previous variable */
         if sharedKind \= 0 then do
            preCount = preCount + 1
            preKind[preCount] = sharedKind
            preText[preCount] = sharedText
            if debug=9 then call log "INJECT SHARED PRESTART=("sharedKind","sharedText")"
            sharedKind = 0
            sharedText = ""
         end

         /* ----------------------------------------------------------
          * Normalise overlay-style prestart:
          *   ABS n, LIT s, ABS n, VAR
          *
          * The trailing ABS n must not reset the cursor again for the
          * variable start, otherwise the located literal anchor is lost.
          * Example:
          *   1 ' go' 1 xx
          * behaves as:
          *   1 ' go' xx
          * ---------------------------------------------------------- */
         if preCount >= 3 then do
            if preKind[preCount] = 3 & preKind[preCount-1] = 2 & preKind[preCount-2] = 3 then do
               if preText[preCount] = preText[preCount-2] then do
                  if debug=9 then call log "NORMALISE PRESTART: DROP TRAILING ABS ("preKind[preCount]","preText[preCount]")"
                  preCount = preCount - 1
               end
            end
         end

         /* apply all buffered controls that define the variable start */
         do j = 1 to preCount

            /* kind 8 = shared literal start:
             * the literal must already sit immediately before the cursor
             */
            if preKind[j] = 8 then do
               p = cursor - length(preText[j])
               if p >= 1 & substr(src, p, length(preText[j])) = preText[j] then
                  cursor = p
               else
                  cursor = src_len + 1
            end

            /* literal search used as a start control */
            else if preKind[j] = 2 then do
               p = pos(preText[j], src, cursor)
               if p = 0 then cursor = src_len + 1
               else do
                  anchorAtStart = 0
                  startsWithBlank = 0

                  if length(preText[j]) > 0 then do
                     if substr(preText[j],1,1) = ' ' then startsWithBlank = 1
                  end

                  /* literal followed by relative motion in prestart:
                   * anchor at literal start so relative motion can apply
                   */
                  if j < preCount then do
                     if preKind[j+1] = 4 | preKind[j+1] = 5 then anchorAtStart = 1
                  end

                  /* non-blank literal followed by +n after the variable:
                   * also anchor at start of match
                   */
                  if anchorAtStart = 0 & startsWithBlank = 0 then do
                     if i < plans then do
                        if planKind[i+1] = 4 then anchorAtStart = 1
                     end
                  end

                  if anchorAtStart then cursor = p
                  else cursor = p + length(preText[j])
               end
            end

            /* numeric/implicit pre-start controls use normal cursor logic */
            else do
               cursor = apply_item_to_cursor(src, src_len, cursor, preKind[j], preText[j])
            end

            if debug=9 then call log "APPLY PRESTART["j"]=("preKind[j]","preText[j]") -> CURSOR="cursor
         end

         preCount = 0
         pendingVar = text
         captureStart = cursor
         iterate
      end

     /* --------------------------------------------------------------
      * No pending variable:
      *   buffer control items until a variable arrives.
      * Pending variable:
      *   current item terminates that variable.
      * -------------------------------------------------------------- */
     if pendingVar = "" then do
        preCount = preCount + 1
        preKind[preCount] = kind
        preText[preCount] = text
        if debug=9 then call log "PRESTART["preCount"]=("kind","text")"
        iterate
     end

     select
        when kind = 2 then do
           /* literal terminates current variable */
           final_cursor = finalize_literal(src, src_len, captureStart, pendingVar, text, rs, cursor, debug)

           /* shared-literal case:
            *   p ',' q +1 r
            * q must start at the same comma that ended p
            */
            if i + 2 <= plans then do
                 if planKind[i+1] = 1 & planKind[i+2] = 4 then do
                   sharedKind = 8
                    sharedText = text
                    if debug=9 then call log "SHARE LITERAL TO NEXT VAR=("sharedKind","sharedText")"
                 end
            end
        end

        when kind = 3 | kind = 4 | kind = 5 then do
           /* numeric boundary terminates current variable */
           final_cursor = finalize_numeric(src, src_len, cursor, captureStart, pendingVar, kind, text, rs, debug)

           /* shared absolute boundary:
            *   ... 3 w2 3 w3
            * the final ABS 3 also becomes the start of w3
            */
           if kind = 3 then do
              if i + 1 <= plans then do
                 if planKind[i+1] = 1 then do
                    sharedKind = 3
                    sharedText = text
                    if debug=9 then call log "SHARE ABS TO NEXT VAR=("sharedKind","sharedText")"
                 end
              end
           end
        end

        when kind = 6 then do
           /* explicit implicit-word boundary */
           final_cursor = finalize_implicit(src, src_len, captureStart, pendingVar, rs, cursor, debug)
        end

        otherwise do
           if debug=9 then call log "parseexec2 error: invalid item kind="kind" at item "i
           final_cursor = cursor
        end
     end

     cursor = final_cursor
     pendingVar = ""
     captureStart = 0
  end

  /* trailing variable gets the remainder of the source */
  if pendingVar \= "" then do
     if captureStart > src_len then value = ""
     else value = substr(src, captureStart)
     if debug=9 then call log ">SEQ ASSIGN REST "pendingVar"='"value"' FROM="captureStart
     rs[rs[0]+1] = value
  end
return rs

/* ----------------------------------------------------------------------
 * decode_stream_plan
 *
 * Decode compiler stream:
 *   kind,len:text;
 * into planKind[] / planText[] arrays.
 * ---------------------------------------------------------------------- */
decode_stream_plan: procedure = .int
  arg planStr=.string, expose planKind=.int[],expose planText=.string[]

  posn = 1
  i = 0

  do while posn <= length(planStr)
     if substr(planStr, posn, 1) = ";" then do
        posn = posn + 1
        iterate
     end

     i = i + 1
     planKind[i] = read_number(planStr, posn)
     call expect_char planStr, posn, ","
     planText[i] = read_lp_field(planStr, posn)
     call expect_char planStr, posn, ";"
  end
return i

/* ----------------------------------------------------------------------
 * apply_item_to_cursor
 *
 * Apply one control item when no variable is currently pending.
 * Used for pre-start cursor positioning.
 * ---------------------------------------------------------------------- */
apply_item_to_cursor: procedure = .int
  arg src=.string, src_len=.int, cursor=.int, kind=.int, text=.string

  p = cursor

  select
     when kind = 2 then do
        /* literal search positions cursor at match start */
        p = pos(text, src, cursor)
        if p = 0 then return src_len + 1
        return p           /* + length(text) */
     end

     when kind = 3 then do
        /* absolute cursor set */
        p = text + 0
        if p < 1 then p = 1
        if p > src_len + 1 then p = src_len + 1
        return p
     end

     when kind = 4 then do
        /* relative forward */
        p = cursor + (text + 0)
        if p < 1 then p = 1
        if p > src_len + 1 then p = src_len + 1
        return p
     end

     when kind = 5 then do
        /* relative backward */
        p = cursor - (text + 0)
        if p < 1 then p = 1
        if p > src_len + 1 then p = src_len + 1
        return p
     end

     when kind = 6 then do
        /* skip blanks to next word start */
        p = cursor
        do while p <= src_len
           if substr(src, p, 1) \= ' ' then leave
           p = p + 1
        end
        return p
     end

     otherwise return cursor
  end
return cursor

/* ----------------------------------------------------------------------
 * finalize_literal
 *
 * Terminate current variable at the next occurrence of a literal.
 * Cursor advances to just after the matched literal.
 * ---------------------------------------------------------------------- */
finalize_literal: procedure=.int
  arg src=.string, src_len=.int, captureStart=.int, pendingVar=.string, lit=.string, expose rs=.string[], cursor=.int, debug=.int

  value=.string
  final_cursor=.int
  p=.int

  /* empty literal splits at current cursor position */
  if length(lit) = 0 then do
     value = ""
     final_cursor = cursor
     if debug=9 then call log ">SEQ ASSIGN EMPTY-LIT "pendingVar"='' FROM="captureStart" NEXTCURSOR="final_cursor
     if pendingVar \= "." then rs[rs[0]+1] = value
     return final_cursor
  end

  p = pos(lit, src, cursor)

  if p = 0 then do
     /* delimiter not found: current variable gets remainder */
     if captureStart > src_len then value = ""
     else value = substr(src, captureStart)
     final_cursor = src_len + 1
     if debug=9 then call log ">SEQ ASSIGN LIT-REST "pendingVar"='"value"' FROM="captureStart
  end
  else do
     if p <= captureStart then value = ""
     else value = substr(src, captureStart, p - captureStart)
     final_cursor = p + length(lit)
     if debug=9 then call log ">SEQ ASSIGN LIT "pendingVar"='"value"' FROM="captureStart" TO="p-1" NEXTCURSOR="final_cursor
  end

  rs[rs[0]+1] = value
return final_cursor

/* ----------------------------------------------------------------------
 * finalize_numeric
 *
 * Terminate current variable using a numeric boundary.
 * Boundary is interpreted from the current runtime cursor, not from the
 * variable capture start.
 * ---------------------------------------------------------------------- */
finalize_numeric: procedure=.int
  arg src=.string, src_len=.int, cursor=.int, captureStart=.int, pendingVar=.string, kind=.int, text=.string, expose rs=.string[], debug=.int

  value=.string
  final_cursor=.int
  p=.int

  if debug=9 then call log "cursor="cursor" captureStart="captureStart" kind="kind" text=<"text"> src_len="src_len

  if kind = 3 then p = text
  else if kind = 4 then p = cursor + text
  else if kind = 5 then p = cursor - text
  else p = cursor

  if p < 1 then p = 1
  if p > src_len + 1 then p = src_len + 1

  if p <= captureStart then do
     /* backward/non-advancing boundary fallback */
     if kind = 5 then do
        /* backward fallback preserves blanks around the word */
        value = take_raw_word(src, captureStart, src_len)
        if debug=9 then call log ">SEQ ASSIGN RAWWORD "pendingVar"='"value"' FROM="captureStart" BOUNDARY="p
     end
     else do
        /* forward/non-advancing fallback takes one plain word */
        value = take_word(src, captureStart, src_len)
        if debug=9 then call log ">SEQ ASSIGN WORD "pendingVar"='"value"' FROM="captureStart" BOUNDARY="p
     end
     final_cursor = captureStart
  end
  else do
     value = substr(src, captureStart, p - captureStart)
     final_cursor = p
     if debug=9 then call log ">SEQ ASSIGN NUM "pendingVar"='"value"' FROM="captureStart" TO="p-1" NEXTCURSOR="final_cursor
  end

  rs[rs[0]+1] = value
return final_cursor

/* ----------------------------------------------------------------------
 * finalize_implicit
 *
 * Terminate current variable by taking one implicit word.
 * Leading blanks are skipped; exactly one separator blank is consumed.
 * ---------------------------------------------------------------------- */
finalize_implicit: procedure=.int
  arg src=.string, src_len=.int, captureStart=.int, pendingVar=.string, expose rs=.string[], cursor=.int, debug=.int

  value=.string
  final_cursor=.int
  p1 = captureStart

  do while p1 <= src_len
     if substr(src, p1, 1) \= ' ' then leave
     p1 = p1 + 1
  end

  if p1 > src_len then do
     value = ""
     final_cursor = src_len + 1
  end
  else do
     p2 = p1
     do while p2 <= src_len
        if substr(src, p2, 1) = ' ' then leave
        p2 = p2 + 1
     end
     value = substr(src, p1, p2 - p1)

     if p2 <= src_len & substr(src, p2, 1) = ' ' then final_cursor = p2 + 1
     else final_cursor = p2
  end

  if debug=9 then call log ">SEQ ASSIGN IMPLICIT "pendingVar"='"value"' FROM="captureStart" NEXTCURSOR="final_cursor
  rs[rs[0]+1] = value
return final_cursor

/* ----------------------------------------------------------------------
 * take_raw_word
 *
 * Take one word from field_start, preserving leading and trailing blanks
 * that belong to that word fragment.
 * ---------------------------------------------------------------------- */
take_raw_word: procedure = .string
  arg src=.string, field_start=.int, src_len=.int

  p1 = field_start
  p2 = p1

  if p2 > src_len then return ""

  do while p2 <= src_len
     if substr(src, p2, 1) \= ' ' then leave
     p2 = p2 + 1
  end

  if p2 > src_len then return substr(src, p1)

  p3 = p2
  do while p3 <= src_len
     if substr(src, p3, 1) = ' ' then leave
     p3 = p3 + 1
  end

  p4 = p3
  do while p4 <= src_len
     if substr(src, p4, 1) \= ' ' then leave
     p4 = p4 + 1
  end
return substr(src, p1, p4 - p1)

/* ----------------------------------------------------------------------
 * take_word
 *
 * Extract exactly one word starting at (or after) field_start.
 * - skips leading blanks
 * - returns word only (no trailing blanks)
 * ---------------------------------------------------------------------- */
take_word: procedure = .string
  arg src=.string, field_start=.int, src_len=.int

  p1 = field_start

  /* skip leading blanks */
  do while p1 <= src_len
     if substr(src, p1, 1) \= ' ' then leave
     p1 = p1 + 1
  end

  /* no word found */
  if p1 > src_len then return ""

  /* scan word */
  p2 = p1
  do while p2 <= src_len
     if substr(src, p2, 1) = ' ' then leave
     p2 = p2 + 1
  end
return substr(src, p1, p2 - p1)

/* ----------------------------------------------------------------------
 * retrieve number from string
 * ---------------------------------------------------------------------- */
read_number: procedure=.string
  arg s=.string, expose posn=.int
  n = ""
  nlen = length(s)
  do while posn <= nlen
     ch = substr(s, posn, 1)
     if ch < "0" | ch > "9" then leave
     n = n || ch
     posn = posn + 1
  end
return n

/* ----------------------------------------------------------------------
 * retrieve field from string
 * ---------------------------------------------------------------------- */
read_lp_field: procedure=.string
  arg s=.string, expose posn=.int
  lenstr = ""
  nlen = length(s)
  do while posn <= nlen
     ch = substr(s, posn, 1)
     if ch = ":" then leave
     lenstr = lenstr || ch
     posn = posn + 1
  end
  posn = posn + 1
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
     call log "decode_stream_plan error: expected '"wanted"' at position" posn "got '"ch"'"
  end
  posn = posn + 1
return

/* ----------------------------------------------------------------------
 * Dump received PARSE plan from compiler exit
 * ---------------------------------------------------------------------- */
dump_stream_plan: procedure
  arg planKind=.int[],planText=.string[]
  do i = 1 to planKind.0
     call log "PLAN["i"] KIND=("planKind[i]","planText[i]")"
  end
return

log: procedure
  arg logtxt = .string
  say ">DBG "logtxt
return