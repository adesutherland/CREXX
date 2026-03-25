options levelb

namespace rxfnsb expose parseExec


/* ----------------------------------------------------------------------
 * parseexec
 *
 * Execute previously normalised PARSE template tokens against a source
 * string and generate replacement assignment statements.
 *
 * Inputs:
 *   src             source string to be parsed
 *   token_type[]    normalised token classes
 *   token[]         normalised token payloads
 *
 * Token classes:
 *   1  receiving variable name (identifier)
 *   2  quoted string / delimiter literal
 *   3  absolute position (plain integer)
 *   4  relative forward shift (+ integer)
 *   5  relative backward shift (- integer)
 *
 * Runtime model:
 *   The token stream is interpreted as:
 *
 *      boundary  target  next-boundary  ...
 *
 *   where:
 *      boundary      = absolute / relative / literal delimiter
 *      target        = receiving variable or "."
 *      next-boundary = determines where current target ends
 *
 * Semantics:
 *   - token[i]     = current boundary
 *   - token[i+1]   = current target variable (or ".")
 *   - token[i+2]   = next boundary used to determine end position
 *
 * The function does not assign variables directly. Instead it builds
 * an array of results per target variable.
 *
 *      a='foo'; b='bar';
 *
 * This can later be injected/executed by the caller.
 *
 * Notes:
 *   - "." acts as a drop target: the parsed value is ignored
 *   - if no next boundary exists, the target receives the remainder
 *   - if a literal delimiter is not found, the remainder is taken
 * ----------------------------------------------------------------------
 */
parseexec: procedure=.string[]
  arg src=.string, type=.string, tokens=.string

  parse_values=.string[]
  token_type=.int[]
  token=.string[]
  wrds=words(type)
  do i=1 to wrds
     token_type[i]=word(type,i)
  ##   say "type["i"]" token_type[i]
  end

  wrds=words(tokens)
  do i=1 to wrds
     token[i]=word(tokens,i)
  ##  say "token["i"]" token[i]
  end

  tkindx=0
  srclen = length(src)
  scan_pos = 1
  i = 1
  vi = 0
  call log "PARSE STRING='"src"'"
  call log "              1---5----0----5----0----5----0----5----0----5----0"

  do while i <= token_type[0]
     t = token_type[i]
     /* --------------------------------------------------------------
      * Debug: current triple = boundary, target, next boundary
      * --------------------------------------------------------------
      */
     call log "DBG i="i " boundary=["token[i]"] type="token_type[i] " var=["token[i+1]"]"
     if i + 2 <= token_type[0] then call log "DBG next boundary=["token[i+2]"] type="token_type[i+2]
     else call log "DBG next boundary=<END>"
     /* --------------------------------------------------------------
      * Current token must be a valid boundary
      * --------------------------------------------------------------
      */
     if t \= 3 & t \= 2 & t \= 4 & t \= 5 & t \= 6 then do
        say "PARSE V1 error: boundary expected at token" i "got type" t "text=<"token[i]">"
        return parse_values
     end
     /* --------------------------------------------------------------
      * Boundary must be followed by a receiving target
      * --------------------------------------------------------------
      */
     if i + 1 > token_type[0] then do
        say "PARSE V1 error: variable expected after boundary at token" i
        return parse_values
     end
     if token_type[i + 1] \= 1 then do
        say "PARSE V1 error: variable expected after boundary at token" i "got type" token_type[i + 1]
        return parse_values
     end
     tkindx=parse_values[0]+1
     varname = token[i + 1]
     vi=vi+1
     call log "*VARIABLE["vi"]="varname" TEMPLATE TOKEN="i+1
     /* --------------------------------------------------------------
      * Resolve current boundary into absolute start position
      *
      * For:
      *   3  -> absolute position
      *   2  -> literal found in source
      *   4  -> relative +n
      *   5  -> relative -n
      *   6  -> implicit (next available position)
      * --------------------------------------------------------------
      */
      call log "DBG current scan_pos="scan_pos " current type="t
      curr_start = resolveCurrentStartV1(t, token[i], scan_pos, src, srclen)
      call log "DBG resolved startpos="curr_start
     /* --------------------------------------------------------------
      * Invalid / unresolved start position:
      * skip current target and continue with next boundary pair
      * --------------------------------------------------------------
      */
     if curr_start < 1 then do
        i = i + 2
        iterate
     end
     /* --------------------------------------------------------------
      * No following boundary:
      * target receives the remainder of the source string
      * --------------------------------------------------------------
      */
     if i + 2 > token_type[0] then do
        value = substr(src, curr_start)
        call log ">ASSIGN "varname"='"value"' FROM="curr_start" LENGTH="length(value)" RESULT-ENTRY="tkindx" >4"
        if varname = '.' then nop
        else parse_values[tkindx] = strip(value)
        scan_pos = srclen + 1
        leave
     end
     nextt = token_type[i + 2]
     nextx = strip(token[i + 2])
     call log "DBG ADDRESS NEXT TOKEN="nextx" TYPE="nextt
     next_start = .int
     /* --------------------------------------------------------------
      * Resolve next boundary if it is numeric / positional
      *
      *   type 3 -> next absolute position
      *   type 4 -> current start + offset
      *   type 5 -> current start - offset
      *   type 6 -> start not available, must be calculated
      *
      * If successful, current target ends just before next_start.
      * --------------------------------------------------------------
      */
     if nextt = 3 then next_start = nextx
     else if nextt = 4 then next_start = curr_start + nextx
     else if nextt = 5 then do
        next_start = curr_start - nextx
        if next_start < 1 then next_start = 1
     end
     else next_start = -1
     /* --------------------------------------------------------------
      * Numeric / positional next boundary:
      * current field runs from curr_start to next_start - 1
      * --------------------------------------------------------------
      */
     if next_start > 0 then do
        endpos = next_start - 1
        if endpos < curr_start then value = ""
        else value = substr(src, curr_start, endpos - curr_start + 1)
        call log ">ASSIGN "varname"='"value"' FROM="curr_start" TO="endpos" LENGTH="length(value)" RESULT-ENTRY="tkindx" >1"
        if varname = '.' then nop
        else parse_values[tkindx] = strip(value)
        scan_pos = next_start
        i = i + 2
        iterate
     end
     /* --------------------------------------------------------------
      * Literal next boundary:
      * search for literal starting at curr_start
      *
      * If found:
      *   target receives text up to, but not including, the literal
      *
      * If not found:
      *   target receives the remainder
      * --------------------------------------------------------------
      */
     if nextt = 2 then do
        p = pos(nextx, src, curr_start)
        if p = 0 then do
           value = substr(src, curr_start)
           call log ">ASSIGN "varname"='"value"' FROM="curr_start" LENGTH="length(value)" RESULT-ENTRY="tkindx" >2"
           if varname = '.' then nop
           else parse_values[tkindx] = strip(value)
           scan_pos = srclen + 1
           leave
        end
        else do
           if p <= curr_start then value = ""
           else value = substr(src, curr_start, p - curr_start)
           call log ">ASSIGN "varname"='"value"' FROM="curr_start" TO="p" LENGTH="length(value)" RESULT-ENTRY="tkindx" >3"
           if varname = '.' then nop
           else parse_values[tkindx] = strip(value)
           scan_pos = p + length(nextx)
           i = i + 2
           iterate
        end
     end

  /* implicit next field boundary */
    if nextt = 6 then do
       rest  = substr(src, curr_start)
       value = word(rest, 1)
       call log ">ASSIGN "varname"='"value"' FROM="curr_start" LENGTH="length(value)" RESULT-ENTRY="tkindx" >5"
       if varname = '.' then nop
       else parse_values[tkindx] = strip(value)
       scan_pos = curr_start + length(value)      /* move to char after extracted word */
       scan_pos=scan_pos-1
       assembler FNDNBLNK scan_pos,src,scan_pos
       scan_pos=scan_pos+1
       i = i + 2
       call log "DBG next scan position "scan_pos" next token "i
       iterate
    end
    say "PARSE V1 error: invalid next boundary type at token" i + 2
    return parse_values
  end
return parse_values

/* ----------------------------------------------------------------------
 * resolveCurrentStartV1
 *
 * Resolve current boundary into absolute start position.
 *
 * Inputs:
 *   t         token type
 *   tok       token payload (token[i])
 *   scan_pos  current scan position
 *   src       source string
 *   srclen    length of source
 *
 * Output:
 *   curr_start (1-based)
 * ---------------------------------------------------------------------- */
resolveCurrentStartV1: procedure = .int
  arg t=.int, tok=.string, scan_pos=.int, src=.string, srclen=.int
  curr_start = -1
  if t = 6 then do
     scan_pos=scan_pos-1
     assembler FNDNBLNK curr_start,src,scan_pos
     curr_start=curr_start+1
     return curr_start
  end
  if t = 4 then do
     curr_start = scan_pos + tok
     if curr_start < 1 then return 1
     if curr_start > srclen + 1 then return srclen + 1
     return curr_start
  end
  if t = 5 then do
     curr_start = scan_pos - tok
     if curr_start < 1 then return 1
     if curr_start > srclen + 1 then return srclen + 1
     return curr_start
  end
  /* fallback: absolute / literal */
return resolveStartV1(src, srclen, tok, t)

    /* ----------------------------------------------------------------------
     * resolveStartV1
     *
     * Convert current boundary token into a 1-based start position.
     *
     * TK_ABS:
     *   token text is numeric column number
     *
     * TK_LIT:
     *   token text is delimiter; start is first char after the literal
     * ---------------------------------------------------------------------- */
    resolveStartV1: procedure=.int
      arg src=.string, srclen=.int, tok=.string, toktype=.int

   tok = strip(tok)
   p=.int
   call log "RESOLVE tok=["tok"] type="toktype " srclen="srclen
   if toktype = 3 then do
      p = tok
      call log "RESOLVE ABSOLUT OFFSET="p
      if p < 1 then return 1
      if p > srclen + 1 then return srclen + 1
      return p
   end

   if toktype = 2 then do
      p = pos(tok, src)
      call log "RESOLVE LIT p="p
      if p = 0 then return -1
      return p + length(tok)
   end
   return -1
log: procedure
    arg logtxt = .string
 ##   say time()" "logtxt
return
