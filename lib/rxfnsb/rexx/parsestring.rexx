/* rexx */
options levelb

namespace rxfnsb expose parseString

/* ----------------------------------------------------------------------
 * Parse the string, using the found tokens and types
 *   parse_string      – the source to parse
 *   token.            – compiled template tokens (literal content for type=2)
 *   token_type.       – 1=VAR, 2=LIT, 3=ABS POS, 4=REL POS, 5=UNQUOTED BLANK, 6=SEARCH (custom)
 * Outputs (aligned by variable index j):
 *   variable.         – variable names in template order (for type=1)
 *   variable_content. – matched field values aligned with variable.
 * Notes:
 *   - Assignments always go to the VAR that most recently opened a field.
 *   - token_lastpos.j records the absolute start of the slice used for variable j
 *     (0 if the remainder was dumped because no explicit locator was found).
 * ---------------------------------------------------------------------- */
parsestring: procedure
  arg parse_string=.string, tokenhi=.int, token=.string[], token_type=.string[], ,
      expose variable=.string[], expose variable_content=.string[], template="unknown"

  /* treat these as whitespace for type=5 */
  WHITESPACE = ' '||'09'x||'0D'x||'0A'x||'0B'x||'0C'x||'A0'x

  L        = length(parse_string)
  pointer  = 1                   /* absolute read pointer (1-based)           */
  j        = 0                   /* variable index                            */

  /* Ownership state for the currently open VAR */
  pendingJ      = 0              /* variable index currently being filled     */
  pendingK      = 0              /* token index of that VAR (debug/help)      */
  pendingStart  = 0              /* absolute start of the current field       */
  newpos        = 0
  beforeptr     = 0
  debug = 0       /* 11 detailed debug, 10 just variable setings */

  /* Optional “allow one blank” start for final fallback */
  finalAfterOneBlank = 0

  /* diagnostics per variable */
    if debug >= 10 then do
       say copies('-',32)
       say '++ String to Parse "'parse_string"'"
       say '++ Template "'template"'"
    end

   do zi = 1 to tokenhi
      token_lastpos.zi = 0
      if debug = 11 then say 'Tokens 'zi "'"token.zi"'" token_type.zi "'"parse_string"'"
   end

  /* ---------- main loop ---------- */
  do k = 1 to tokenhi
     type = token_type.k
     if debug = 11 then say 'Fetch next token, Index 'k', Token "'token.k'" type 'type
/* -------------------------------------------------------------------
 * VAR opens a new field
 * -------------------------------------------------------------------
 */
     if type = 1 then do
        if debug = 11 then say 'entering Type 'type 'var-number 'j' Pointer 'pointer
        j = j + 1
        variable.j = token.k
        pendingJ     = j
        pendingK     = k
        pendingStart = pointer
        iterate
     end
/* -------------------------------------------------------------------
 * ABS/REL positions close the pending field
 * -------------------------------------------------------------------
 */
     else if type = 3 | type = 4 then do
       if beforeptr>0  then do
          if debug = 11 then say '** entering Type 'type' Current Pointer 'pointer' has been reset to 'beforeptr
          pointer=beforeptr
          pendingStart=pointer
          beforeptr=0
        end

        newpos=0
        newpos = token.k
        if type = 4 then newpos = pointer + newpos
        if newpos < 1 then newpos = 1
        if newpos > L + 1 then newpos = L + 1

        if debug = 11 then say 'entering Type 'type' Current Pointer 'pointer' set Pointer to 'token.k' new Pointer 'newpos
        if pendingJ > 0 then do
           token_lastpos.pendingJ   = pendingStart
           if newpos <= pendingStart then do
              variable_content.pendingJ=substr(parse_string, pendingStart)
              if debug >= 10 then say '>> Pending Variable 'variable.pendingJ'="'variable_content.pendingJ'" (remainder from 'effStart' due to backward move)'
           end
           else do
              variable_content.pendingJ = substr(parse_string, pendingStart, newpos - pendingStart)
              if debug >= 10 then say '>> Pending Variable 'variable.pendingJ'="'variable_content.pendingJ'" [slice 'pendingStart'..'newpos-pendingStart']'
           end
           pendingJ = 0 ; pendingK = 0 ; pendingStart = 0; beforeptr=0
        end
        pointer = newpos
        if debug = 11 then say 'Leaving Type 'type' New Pointer 'pointer' pointing at "'substr(parse_string,pointer)'"'
        iterate
     end
/* -------------------------------------------------------------------
 * LIT delimiter closes the pending field
 * -------------------------------------------------------------------
 */
     else if type = 2 then do
        chunk = substr(parse_string, pointer)
        p = pos(token.k, chunk)  /* position of delimiter relative to pointer */
        if debug = 11 then say 'entering Type 'type' Current Pointer 'pointer' Remaining String "'chunk'" Search pattern "'token.k'" located at 'p
        if p = 0 then do
           /* not found: dump remainder to pending VAR */
           if pendingJ > 0 then do
              token_lastpos.pendingJ    = 0
              variable_content.pendingJ = substr(parse_string, pendingStart)
              if debug >= 10 then say '>> Variable Set Type 'type' Variable 'variable.pendingJ'="'variable_content.pendingJ'"'
              pendingJ = 0 ; pendingK = 0 ; pendingStart = 0 ; beforeptr=0
           end
           pointer = L + 1
        end
        else do
           if pendingJ > 0 then do
              token_lastpos.pendingJ    = pendingStart
              /* slice up to char before delimiter */
              variable_content.pendingJ = substr(parse_string, pendingStart, (pointer + p - 1) - pendingStart)
              if debug >= 10 then say '>> Variable Set Type 'type' Variable 'variable.pendingJ'="'variable_content.pendingJ'"'
              pendingJ = 0 ; pendingK = 0 ; pendingStart = 0 ; beforeptr=0
           end
           beforeptr = pointer + p  -1   ## just keep beforeptr if there was a variable set
           /* hop past the delimiter */
           pointer = pointer + p - 1 + length(token.k)
        end
        if debug = 11 then do
           say 'Leaving Type 'type' New Pointer 'pointer' pointing at "'substr(parse_string,pointer)'"'
           if beforeptr>0 then say '.... Alternatively New Pointer 'beforeptr' pointing at "'substr(parse_string,beforeptr)'"'
        end
        iterate
     end
/* -------------------------------------------------------------------
 * UNQUOTED BLANK closes the pending field
 * -------------------------------------------------------------------
 */
     else if type = 5 then do
     /* drop any leading blank */
        do while pointer<=L
           if pos(substr(parse_string,pointer, 1), WHITESPACE)=0 then leave
           pointer=pointer+1
        end
        PendingStart=pointer
        chunk = substr(parse_string, pointer)
        lenC  = length(chunk)
        /* find first whitespace from current pointer */
        if debug = 11 then say 'entering Type 'type' Current Pointer 'pointer' Remaining String "'chunk'" Find first WS'

        p = 0
        do xi = 1 to lenC
           if pos(substr(chunk, xi, 1), WHITESPACE) > 0 then do
              p = xi
              leave
           end
        end
        if p = 0 then do
           /* no whitespace: dump remainder to pending VAR */
           if pendingJ > 0 then do
              token_lastpos.pendingJ    = 0
              variable_content.pendingJ = substr(parse_string, pendingStart)
              if debug >= 10 then say '>> Pending Variable 'variable.pendingJ'="'variable_content.pendingJ'" Remaing'
              pendingJ = 0 ; pendingK = 0 ; pendingStart = 0
           end
           pointer = L + 1
        end
        else do
           if pendingJ > 0 then do
              token_lastpos.pendingJ    = pendingStart
              variable_content.pendingJ = substr(parse_string, pendingStart, (pointer + p - 1) - pendingStart)
              if debug >= 10 then say '>> Pending Variable 'variable.pendingJ'="'variable_content.pendingJ'" from 'PendingStart' up to 'pointer + p - 1
              pendingJ = 0 ; pendingK = 0 ; pendingStart = 0 ; beforeptr=0
           end

           /* record “allow one blank” position for final fallback (after first blank) */
           finalAfterOneBlank = pointer + p

           /* skip the entire whitespace run (treat as a single blank for parsing) */
           runEnd = p
           do while runEnd <= lenC & pos(substr(chunk, runEnd, 1), WHITESPACE) > 0
              runEnd = runEnd + 1
           end
           pointer = pointer + runEnd - 1
        end
        if debug = 11 then say 'Leaving Type 'type' New Pointer 'pointer' pointing at "'substr(parse_string,pointer)'"'
        iterate
     end
/* -------------------------------------------------------------------
 * Variable search of already found token
 * -------------------------------------------------------------------
 */
     else if type = 6 then do
          if debug = 11 then say 'entering Type 'type' Search for already set Variable 'token.k
          jj=0
          do ii=1 to k-1
             if token_type.ii\=1 then iterate
             jj=jj+1
             if token.ii=token.k then leave
          end
          if token.ii \= token.k then iterate
          chunk = substr(parse_string, pointer)
          p = pos(variable_content.jj, chunk)  /* position of delimiter relative to pointer */
          if p = 0 then do
           /* not found: dump remainder to pending VAR */
             if pendingJ > 0 then do
                token_lastpos.pendingJ    = 0
                variable_content.pendingJ = substr(parse_string, pendingStart)
                if debug >= 10 then say '>> Pending Variable 'variable.pendingJ'="'variable_content.pendingJ'"'
                pendingJ = 0 ; pendingK = 0 ; pendingStart = 0 ; beforeptr=0
             end
             pointer = L + 1
          end
          else do
             if pendingJ > 0 then do
             /* slice up to char before delimiter */
             variable_content.pendingJ = substr(parse_string, pendingStart, (pointer + p - 1) - pendingStart)
             if debug >= 10 then say '>> Pending Variable 'variable.pendingJ'="'variable_content.pendingJ'"'
             pendingJ = 0 ; pendingK = 0 ; pendingStart = 0 ; beforeptr=0
          end
          /* hop past the delimiter */
           pointer = pointer + p - 1 + length(variable_content.jj)
        end
        if debug = 11 then say 'Leaving Type 'type' New Pointer 'pointer' pointing at "'substr(parse_string,pointer)'"'
        iterate
       end
  end  /* k loop */

  /* ---- Final fallback: if a VAR is still open, assign the tail ---- */

  if pendingJ > 0 then do
     /* For your original “allow 1 blank” semantics, prefer the position just after the first blank
        found by a prior type-5; otherwise use the current pointer. */
     startPos = pointer
     if finalAfterOneBlank > 0 then startPos = finalAfterOneBlank
     if startPos <= L then variable_content.pendingJ = substr(parse_string, startPos)
                      else variable_content.pendingJ = ''
     token_lastpos.pendingJ = startPos
     if debug >= 10 then say '>> Pending Variable 'variable.pendingJ'="'variable_content.pendingJ'"'
     pendingJ = 0 ; pendingK = 0 ; pendingStart = 0
  end
return