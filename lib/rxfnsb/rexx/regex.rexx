/* rexx */
options levelb

namespace rxfnsb expose regexmatch REGEXFIND REGEXSPLIT REGEXTEST REGEXFINDALL REGEXREPLACE REGEXREPLACE_LIMIT REGEXDETAILS REGEXSUBSTR rxlite_start rxlite_len rxlite_end rxlite_ci
/* ----------------------------------------------------------------------------
 * REGEXMATCH external  0: s doesn't contain p, 1: p does
 * ----------------------------------------------------------------------------
 */
REGEXMATCH: PROCEDURE=.int
  arg s=string, p=.string
  rxc=__regexmatch(s,p)
return rxc
/* ----------------------------------------------------------------------------
 * __REGEXMATCH internal  * REGEXMATCH 0: s doesn't contain p, 1: p does
 * ----------------------------------------------------------------------------
 */
__REGEXMATCH: PROCEDURE=.int
  arg s=string, p=.string
  i = 0
  a = 0
  q = ""

  rxlite_start = 0
  rxlite_len   = 0
  rxlite_end   = 0
  rxlite_ci = 0
  if left(p,4) = '(?i)' then do
      rxlite_ci = 1
      p = substr(p,5)
  end
  /* --- TOP-LEVEL ALTERNATION --- */
  alt=__altSplit(p)
  if alt.0 > 1 then do
  /* Try each alternative with current CI flag. ^ inside an alt applies to that alt only. */
     do a = 1 to alt.0
        q = alt.a
        /* anchored alt? */
        if left(q,1) = '^' then do
           if __matchHere(s, 1, q, 2) then do
              rxlite_start = 1
              return 1
           end
        end
        else do
           do i = 1 to length(s) + 1
              if __matchHere(s, i, q, 1) then do
                 rxlite_start = i
                 return 1
              end
           end
        end
     end
     return 0
  end
  /* anchored-at-start? */
  if left(p,1) = '^' then do
    if __matchHere(s, 1, p, 2)=0 then return 0
    rxlite_start = 1
    return 1
  end
  /* try every start position */
  do i = 1 to length(s) + 1
     if __matchHere(s, i, p, 1) then do
        rxlite_start = i
        return 1
    end
  end
return 0
/* ----------------------------------------------------------------------------
 * REGEXFIND REGEXFIND(s, p, fromPos) -> 1/0; sets rxlite_start, rxlite_len
 * ----------------------------------------------------------------------------
 */
REGEXFIND: PROCEDURE=.int
  arg s=string, p=.string, fromPos=.int
  i = 0
  a = 0
  q = ""

  if fromPos = '' then fromPos = 1

  rxlite_start = 0
  rxlite_len   = 0
  rxlite_end   = 0
  if p \= '' then do
     rxlite_ci = 0
     if left(p,4) = '(?i)' then do
        rxlite_ci = 1
        p = substr(p,5)
     end
  end
   /* === BEGIN: TOP-LEVEL ALTERNATION === */

    alt = __altSplit(p)

    if alt.0 > 1 then do
      /* Scan positions; try each alt at each position.
         For '^' alts, only try at i=1. */
       do i = fromPos to length(s) + 1
          do a = 1 to alt.0
             q = alt.a
             if left(q,1) = '^' then do
                if i \= 1 then iterate
                if __matchHere(s, 1, q, 2) then do
                   rxlite_start = 1
                   rxlite_len   = rxlite_end - 1
                   if rxlite_len < 0 then rxlite_len = 0
                   return 1
                end
             end
             else do
                if __matchHere(s, i, q, 1) then do
                   rxlite_start = i
                   rxlite_len   = rxlite_end - i
                   if rxlite_len < 0 then rxlite_len = 0
                   return 1
                end
             end
          end
       end
       return 0
    end
    /* === END: TOP-LEVEL ALTERNATION === */

  /* anchored ^ only valid at pos 1 */
  if left(p,1) = '^' then do
     if fromPos \= 1 then return 0
     if __matchHere(s, 1, p, 2) then do
        rxlite_start = 1
        rxlite_len   = rxlite_end - 1
        if rxlite_len < 0 then rxlite_len = 0
        return 1
     end
     return 0
  end
/* ---- FAST-PATH for a single character class with no anchors/quantifiers ---- */
  if left(p,1)='[' & right(p,1)=']' then do
  /* ensure it’s exactly one class atom (no trailing tokens) */
  /* PARSE value __readClass(p,1) with spec type plen */
     _pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
        _string2Parse=__readClass(p,1)
        _parsetemplate='spec type plen'
        call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0
     ## ---------- set parse variables ----------
     if type = 'CLASS' & plen = length(p) then do
        do i = fromPos to length(s)
           if __classHas(spec, substr(s,i,1)) then do
              rxlite_start = i
              rxlite_len   = 1
              rxlite_end   = i + 1
              return 1
           end
        end
        return 0
     end
  end
/* --------------------------------------------------------------------------- */
  do i = fromPos to length(s) + 1
     if __matchHere(s, i, p, 1) then do
        rxlite_start = i
        rxlite_len   = rxlite_end - i   /* end-after - start */
        if rxlite_len < 0 then rxlite_len = 0
        return 1
     end
  end
return 0
/* ----------------------------------------------------------------------------
 * REGEXSPLIT(s, pat, outStem[, opts, limit]) -> count
 *    - outStem.1..outStem.count filled with pieces
 *    - optional opts: 'T' trim, 'D' drop empties, 'E' expand multi-char delimiters
 *    - optional limit: maximum number of pieces (last piece takes the remainder)
 * ----------------------------------------------------------------------------
 */
REGEXSPLIT: PROCEDURE=.string[]
  arg s=.string, pat=.string, opts='', limit=0
  piece = ""
  count = 0
  /* defaults */
  keepEmpty = 1
  doTrim    = 0
  expand    = 0
  outstem=.string[]
  if opts \= '' then do
     u = translate(opts)
     if pos('T', u) > 0 then doTrim = 1
     if pos('D', u) > 0 then keepEmpty = 0
     if pos('E', u) > 0 then expand   = 1
  end
  count = 0
  pos   = 1
  prev  = 1

  do while REGEXFIND(s, pat, pos)
    /* ensure rxlite_len is set if REGEXFIND doesn't set it itself */
     call regexfinalizeLen s

    /* if we're about to hit the limit, make the last piece absorb the rest */
     if limit > 0 & count = limit - 1 then do
        piece = substr(s, prev)
        if doTrim then piece = strip(piece)
        if keepEmpty | piece \= '' then do
           count = count + 1
           outStem.count = piece
        end
        return outstem
    end
 /* piece before the delimiter */
    piece = substr(s, prev, rxlite_start - prev)
    if doTrim then piece = strip(piece)
    if keepEmpty | piece \= '' then do
       count = count + 1
       outStem.count = piece
    end
    /* optionally expand multi-char delimiter into extra empties */
    if expand & rxlite_len > 1 then do e = 2 to rxlite_len
       if keepEmpty then do
          count = count + 1
          outStem.count = ""       /* extra empty for each extra delimiter char */
          if limit > 0 & count >= limit then return outstem
       end
    end

    /* advance past the match (at least 1 to avoid loops) */
    pos  = rxlite_start + max(1, rxlite_len)
    prev = pos
  end

  /* tail after last delimiter (possibly empty) */
  piece = substr(s, prev)
  if doTrim then piece = strip(piece)
  if keepEmpty | piece \= '' then do
     count = count + 1
     outStem.count = piece
  end
return outstem
/* ----------------------------------------------------------------------------
 * REGEXREPLACE(s, pat, repl) -> new string
 *   Replace all matches. Safe for zero-length patterns (^, $).
 * ----------------------------------------------------------------------------
 */
REGEXREPLACE: PROCEDURE=.string
   arg s=.string, pat=.string, repl=.string

   out  = ''
   pos  = 1
   prev = 1

   ok = REGEXFIND(s, pat, pos)
   do while ok
     call regexfinalizeLen s               /* ensure rxlite_len is valid */

     rep = __expandRepl(repl, s)           /* <<< expand $0 / $& / $$ */

     /* prefix (up to match) + expanded replacement */
     out = out || substr(s, prev, rxlite_start - prev) || rep

     /* advance */
     step = max(1, rxlite_len)
     pos  = rxlite_start + step
     prev = rxlite_start + rxlite_len

     ok = REGEXFIND(s, pat, pos)
   end

   out = out || substr(s, prev)
return out

/* ----------------------------------------------------------------------------
 * REGEXREPLACE_LIMIT(s, pat, repl, limit=0) -> new string
 *   Replace up to 'limit' matches (0 = replace all). Safe for zero-length patterns.
 * ----------------------------------------------------------------------------
 */
/* REGEXREPLACE_LIMIT(s, pat, repl, limit=0) -> new string */
REGEXREPLACE_LIMIT: PROCEDURE=.string
  arg s=.string, pat=.string, repl=.string, limit=0

  out  = ''
  pos  = 1
  prev = 1
  n    = 0

  ok = REGEXFIND(s, pat, pos)
  do while ok & (limit = 0 | n < limit)
    call regexfinalizeLen s

    rep = __expandRepl(repl, s)           /* <<< expand here too */

    out = out || substr(s, prev, rxlite_start - prev) || rep

    step = max(1, rxlite_len)
    pos  = rxlite_start + step
    prev = rxlite_start + rxlite_len

    n  = n + 1
    ok = REGEXFIND(s, pat, pos)
  end

  out = out || substr(s, prev)
return out
/* ----------------------------------------------------------------------------
 * REGEXFINDALL(s, pat, outStart., outLen., outText.[, opts, limit]) -> count
 *   - Collects all matches into the provided stems
 *   - Returns number of matches
 *   - outStart.i = start position (1-based)
 *   - outLen.i   = match length
 *   - outText.i  = matched substring
 *
 * opts:
 *   "O" = allow overlaps (advance by 1 char)
 * limit >0 = stop after that many matches
 * ----------------------------------------------------------------------------
 */
REGEXFINDALL: PROCEDURE=.string[]
  arg s=.string, pat=.string, opts='', limit=0

  allowOverlap = 0
  if pos('O', translate(opts)) > 0 then allowOverlap = 1

  outText=.string[]
  count = 0
  pos   = 1

  ok = REGEXFIND(s, pat, pos)
  do while ok
     call regexfinalizeLen s
     count = count + 1
##   outStart.count = rxlite_start   ## not necessary in the moment, but keep it
##   outLen.count   = rxlite_len
     outText.count  = substr(s, rxlite_start, rxlite_len)

     if limit > 0 & count >= limit then leave

     if allowOverlap then pos = rxlite_start + 1
     else pos = rxlite_start + max(1, rxlite_len)
     ok = REGEXFIND(s, pat, pos)
  end
return outText
/* ----------------------------------------------------------------------------
 * REGEXTEST(text, pattern): say a one-line verdict
 * ----------------------------------------------------------------------------
 */
REGEXTEST: PROCEDURE
  arg s=.string, p=.string
  if __REGEXMATCH(s, p) then do
     call regexfinalizeLen s
     say 'MATCH at' rxlite_start 'len' rxlite_len '-> "'||substr(s, rxlite_start, rxlite_len)||'"'
  end
  else say 'NO MATCH'
return
/* ----------------------------------------------------------------------------
 * RXDETAILS: fill caller stem with last-match info.
 *    outStem.1 = start, outStem.2 = len, outStem.3 = end; outStem.0 = 3
 *    Returns 1 if a match exists, else 0.
 * ----------------------------------------------------------------------------
 */
REGEXDETAILS: PROCEDURE=.int[]
  outStem=.int[]
  call regexfinalizeLen ''          /* finalize internally */

  if rxlite_start = 0 then do
     outStem.1 = 0
     outStem.2 = 0
     outStem.3 = 0
     return outstem
  end

  outStem.1 = rxlite_start
  outStem.2 = rxlite_len
  outStem.3 = rxlite_start + max(1,rxlite_len)
return outstem
/* ----------------------------------------------------------------------------
 * RXSUBSTR(buffer) -> substring of last match in that buffer
 * ----------------------------------------------------------------------------
 */
/*  */
REGEXSUBSTR: PROCEDURE=.string
  arg buffer=.string
  if rxlite_start = 0 | rxlite_len = 0 then return ""
return substr(buffer, rxlite_start, rxlite_len)
/* ----------------------------------------------------------------------------
 * Expand $0 (or $&) to the current match, $$ -> literal $.
 *    Called from inside the replace loop, AFTER regexfinalizeLen.
 * ----------------------------------------------------------------------------
 */
__expandRepl: PROCEDURE=.string
  arg repl=.string, s=.string
  m = ""
  out = ""
  i = 0
  c = ""
  d = ""

  /* finalize length first; rxlite_len may not be set yet */
  call regexfinalizeLen ''

  /* Compute a safe match string m */
  m = ''
  if rxlite_start >= 1 then do
    /* anchors can yield len=0; leave m='' in that case */
    if rxlite_len > 0 then do
      L = length(s)
      /* guard against start > L (e.g., '$' anchor => start=L+1, len=0) */
      if rxlite_start <= L then m = substr(s, rxlite_start, rxlite_len)
    end
  end

  /* Expand $0 / $& to m, $$ to literal $ */
  out = ''
  i = 1
  do while i <= length(repl)
    c = substr(repl, i, 1)
    if c = '$' then do
      d = substr(repl, i+1, 1)
      if d = '0' | d = '&' then do
        out = out || m
        i = i + 2
        iterate
      end
      if d = '$' then do
        out = out || '$'
        i = i + 2
        iterate
      end
    end
    out = out || c
    i = i + 1
  end
return out
/* ----------------------------------------------------------------------------
 * __matchHere:
 * ----------------------------------------------------------------------------
 */
__matchHere: PROCEDURE=.int
  /* Does pattern p[j..] match s starting at i? */
  arg s=.string, i=.int, p=.string, j=.int
  /* 1) End of pattern -> success (record cursor-after) */
  if j > length(p) then do
     call __set_last_end i
     return 1
  end
  /* 2) $ anchor (only at end) */
  if substr(p, j, 1) = '$' & j = length(p) then do
     if i > length(s) then do
        call __set_last_end i
        return 1
     end
     return 0
  end
  /* 3) Read next atom */
  atomLen = 0
  c = substr(p, j, 1)
  if c = '\' then do
/* PARSE value __readEscape(p,j) with atom type atomLen */
   _pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=__readEscape(p,j)
   _parsetemplate='atom type atomLen'
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0

## ---------- set parse variables ----------
atom=_pass_variable_content.1
type=_pass_variable_content.2
atomLen=_pass_variable_content.3
## ---------- parse variables set ----------
  end
  else if c = '[' then do
/* PARSE value __readClass(p,j) with atom type atomLen */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=__readClass(p,j)
   _parsetemplate='atom type atomLen'
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0
## ---------- set parse variables ----------
atom=_pass_variable_content.1
type=_pass_variable_content.2
atomLen=_pass_variable_content.3
## ---------- parse variables set ----------
     if atom = 'ERROR' then return 0
  end
  else do
     atom    = c
     type    = 'ATOM'
     atomLen = 1
  end
  /* 4) Quantifier dispatch */
  q = substr(p, j + atomLen, 1)
  qnext = substr(p, j + atomLen + 1, 1)
    /* * and + (greedy / lazy) */
    if q = '*' then do
       if qnext = '?' then return __matchStarLazy(s, i, p, j, atomLen)
       else               return __matchStar(s,      i, p, j, atomLen)
    end
    else if q = '+' then do
       if __matchOne(s, i, atom, type) then do
          if qnext = '?' then return __matchStarLazy(s, i+1, p, j, atomLen)
          else                return __matchStar(s,      i+1, p, j, atomLen)
      end
      return 0
    end
    else if q = '?' then do
      /* 0 repetitions */
    if __matchHere(s, i, p, j + atomLen + 1) then return 1
    /* 1 repetition */
    if __matchOne(s, i, atom, type) then do
       if __matchHere(s, i+1, p, j + atomLen + 1) then return 1
    end
    return 0
  end
  /* 5) No quantifier: advance and recurse (do NOT set end here) */
  if __matchOne(s, i, atom, type) then return __matchHere(s, i+1, p, j + atomLen)
return 0
/* ----------------------------------------------------------------------------
 * __matchStar
 * ----------------------------------------------------------------------------
 */
__matchStar: PROCEDURE=.int
  arg s=.string, i=.int, p=.string, j=.int, atomLen=.int
  /* Re-read the atom */
  c = substr(p, j, 1)
  if c = '\' then do
/* PARSE value __readEscape(p,j) with atom type . */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=__readEscape(p,j)
_parsetemplate='atom type .'
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0
## ---------- set parse variables ----------
atom=_pass_variable_content.1
type=_pass_variable_content.2
## ---------- parse variables set ----------
  end
  else if c = '[' then do
/* PARSE value __readClass(p,j) with atom type . */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=__readClass(p,j)
_parsetemplate='atom type .'
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0
## ---------- set parse variables ----------
atom=_pass_variable_content.1
type=_pass_variable_content.2
## ---------- parse variables set ----------
     if atom = 'ERROR' then return 0
  end
  else do
     atom = c
     type = 'ATOM'
  end
  /* Greedy consume */
  k = i
  do while __matchOne(s, k, atom, type)
     k = k + 1
  end
  /* Backtrack to find a continuation that matches */
  posAfter = j + atomLen + 1
  do m = k to i by -1
     if __matchHere(s, m, p, posAfter) then return 1
  end
return 0
/* ----------------------------------------------------------------------------
 * MATCHONE
 * ----------------------------------------------------------------------------
 */
__matchOne: PROCEDURE=.int
  arg s=.string, i=.int, atom=.string, type=.string
  ch = ""
  L = length(s)
  if i > L then return 0
  if type = 'CLASS' then do
     ch = substr(s, i, 1)
     return __classHas(atom, ch)
  end

  ch = substr(s, i, 1)
  if atom = '.' then return 1
  if left(atom,1) = '#' then return __escapePredicate(atom, ch)  /* \d \w \s etc. */

  /* literal */
  if rxlite_ci then return (translate(ch) = translate(atom))
return (ch = atom)
/* ----------------------------------------------------------------------------
 * READESCAPE
 * ----------------------------------------------------------------------------
 */
__readEscape: PROCEDURE=.string
  arg p=.string, j=.int
  e = substr(p, j+1, 1)
  if e='d' then return '#d' 'ATOM' 2
  else if e='D' then return '#D' 'ATOM' 2
  else if e='w' then return '#w' 'ATOM' 2
  else if e='W' then return '#W' 'ATOM' 2
  else if e='s' then return '#s' 'ATOM' 2
  else if e='S' then return '#S' 'ATOM' 2
return e 'ATOM' 2          /* escaped literal */
/* ----------------------------------------------------------------------------
 * __ESCAPEPREDICT
 * ----------------------------------------------------------------------------
 */
__escapePredicate: PROCEDURE=.int
  arg code=.string, ch=.string

  if code = '#d' then return (pos(ch,'0123456789')>0)
  else if code = '#D' then return \(__escapePredicate('#d',ch))
  else if  code = '#w' then return (datatype(ch,'A')=1 | datatype(ch,'N')=1 | ch='_')
  else if  code = '#W' then return \(__escapePredicate('#w',ch))
  else if  code = '#s' then do
     ws = ' '||d2c(9)||d2c(10)||d2c(13)||d2c(12)   /* space, \t, \n, \r, \f */
     return (pos(ch, ws) > 0)
  end
  else if code = '#S' then return \(__escapePredicate('#s',ch))
return 0
/* ----------------------------------------------------------------------------
 * __READCLASS
 * ----------------------------------------------------------------------------
 */
/* Parse a [...] character class */
__readClass: PROCEDURE=.string
  arg p=.string, j=.int
  k = j + 1
  neg = 0
  if substr(p, k, 1) = '^' then do
     neg = 1
     k = k + 1
  end

  pairs = ''                                /* store as concatenated lo/hi pairs */
  seen = 0
  do while k <= length(p)
     c = substr(p, k, 1)
     if c = ']' & seen = 1 then do
       classLen = k - j + 1
       return neg'|'pairs 'CLASS' classLen
    end

    /* read one char, honoring escapes */
    if c = '\' then do
       c = __classEsc(substr(p, k+1, 1))
       k = k + 2
    end
    else do
       k = k + 1
    end

    /* range like a-z */
    if k <= length(p)-1 & substr(p, k, 1) = '-' & substr(p, k+1, 1) \= ']' then do
       r2 = substr(p, k+1, 1)
       if r2 = '\' then do
          r2 = __classEsc(substr(p, k+2, 1))
          k  = k + 3
       end
       else k = k + 2
       pairs = pairs || c || r2
       seen = 1
    end
    else do
       pairs = pairs || c || c
       seen = 1
    end
  end
return 'ERROR' 'CLASS' 0                  /* unterminated class */
/* ----------------------------------------------------------------------------
 * __CLASSESCAPE
 * ----------------------------------------------------------------------------
 */
__classEsc: PROCEDURE=.string
  arg e=.string

  if e='n' then return d2c(10)
  else if e='r' then return d2c(13)
  else if  e='t' then return d2c(9)
return e
/* ----------------------------------------------------------------------------
 * __CLASSHAS
 * ----------------------------------------------------------------------------
 */
__classHas: PROCEDURE=.int
  arg spec=.string, ch=.string
  /* PARSE var spec neg '|' pairs */
  _pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
     _string2Parse=spec
     _parsetemplate="neg'|'pairs"
     call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0
  ## ---------- set parse variables ----------
     neg=_pass_variable_content.1
     pairs=_pass_variable_content.2
 ## ---------- parse variables set ----------

    hit = 0
  if rxlite_ci then do
     ch  = translate(ch)
     do i = 1 to length(pairs) by 2
        lo = translate(substr(pairs, i, 1))
        hi = translate(substr(pairs, i+1,1))
        if ch >= lo & ch <= hi then do; hit = 1; leave; end
     end
  end
  else do
    do i = 1 to length(pairs) by 2
       lo = substr(pairs, i, 1)
       hi = substr(pairs, i+1,1)
       if ch >= lo & ch <= hi then do; hit = 1; leave; end
    end
  end
  if neg = '1' then return \hit
return hit
/* ----------------------------------------------------------------------------
 * __matchStarLAZY Lazy star: prefer the shortest match that allows the rest to match
 * ----------------------------------------------------------------------------
 */
__matchStarLazy: PROCEDURE=.int
  arg s=.string, i=.int, p=.string, j=.int, atomLen=.int
  /* re-read the atom (same as in __matchStar) */
  c = substr(p, j, 1)
  if c = '\' then do
/* PARSE value __readEscape(p,j) with atom type . */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=__readEscape(p,j)
_parsetemplate='atom type .'
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0
## ---------- set parse variables ----------
atom=_pass_variable_content.1
type=_pass_variable_content.2
## ---------- parse variables set ----------
  end
  else if c = '[' then do
/* PARSE value __readClass(p,j) with atom type . */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=__readClass(p,j)
_parsetemplate='atom type .'
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,0
## ---------- set parse variables ----------
atom=_pass_variable_content.1
type=_pass_variable_content.2
## ---------- parse variables set ----------
     if atom = 'ERROR' then return 0
  end
  else do
     atom = c
     type = 'ATOM'
  end
  /* compute how far we CAN go */
  k = i
  do while __matchOne(s, k, atom, type)
     k = k + 1
  end
  /* L A Z Y  backtrack: try the shortest first */
  posAfter = j + atomLen + 2   /* skip atom, '*', and the trailing '?' */
  do m = i to k                 /* increasing, not decreasing */
     if __matchHere(s, m, p, posAfter) then return 1
  end
return 0
/* ----------------------------------------------------------------------------
 * __SET_LAST_END single place to store end cursor
 * ----------------------------------------------------------------------------
 */
__set_last_end: PROCEDURE
  arg endIdx=.int
  rxlite_end = endIdx
return

/* ----------------------------------------------------------------------------
 * regexfinalizeLen
 * ----------------------------------------------------------------------------
 */
regexfinalizeLen: PROCEDURE=.int
  arg s=.string
  /* Only recompute len when rxlite_end is available. Otherwise LEAVE the existing rxlite_len intact. */
  if rxlite_end > 0 then rxlite_len = rxlite_end - rxlite_start
  rxlite_end = 0  /* Clear end cursor after consumption */
return rxlite_len
/* ----------------------------------------------------------------------------
 * __altSplit(p) -> count
 *   Split on top-level '|' (ignores \| and any '|' inside [...] classes)
 * ----------------------------------------------------------------------------
 */
__altSplit: PROCEDURE=.string[]
  arg p=.string

  altmax=0
  L   = length(p)
  i   = 1
  segStart = 1
  inClass  = 0
  alts=.string[]

  do while i <= L
     c = substr(p, i, 1)
  /* handle escapes everywhere: skip next char */
     if c = '\' then do
        i = i + 2
        iterate
     end
  /* enter/exit character class */
     if c = '[' then do
        inClass = 1
        i = i + 1
        iterate
     end
     if c = ']' then do
        inClass = 0
        i = i + 1
        iterate
     end
  /* top-level alternation split */
    if c = '|' & inClass = 0 then do
       altmax=altmax+1
       alts[altmax] = substr(p, segStart, i - segStart)
       segStart = i + 1
       i = i + 1
       iterate
    end
    i = i + 1
  end
  /* tail segment */
  altmax=altmax+1
  alts[altmax] = substr(p, segStart)
return alts