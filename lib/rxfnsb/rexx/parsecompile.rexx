/* rexx */
options levelb

namespace rxfnsb expose parseCompile

/* ----------------------------------------------------------------------
 * Compile the given template, split it in tokens and token.ktypes
 * parseCompile
 *  - Tokenizes a template with support for quoted strings ('' and "")
 *  - Handles quotes across spaces and doubled quotes inside strings
 *  - Splits punctuation as separate tokens (tune PUNCT set as needed)
 *  - Classifies tokens:
 *      1 = identifier/word
 *      2 = quoted string
 *      3 = unsigned integer
 *      4 = signed integer (+/- followed by digits)
 *      5 = punctuation/operator
 * ----------------------------------------------------------------------
 */
parseCompile: Procedure=.int
  arg template=.string,expose token=.string[],expose token_type=.string[]
  template=preCleanTemplate(template)
  k       = 0
  buf     = ''                              /* pending unquoted text */
  WHITESPACE = ' '||'09'x||'0D'x||'0A'x||'0B'x||'0C'x||'A0'x

 /* flush pending unquoted text as a token */
  tokenhi=0
  L = length(template)
  i = 1
  do while i <= L
     ch = substr(template, i, 1)
     if pos(ch, WHITESPACE) > 0 then do
        buf=flush(buf,token,tokenhi)
        j = i + 1
        do while j <= L & pos(substr(template, j, 1), WHITESPACE) > 0
           j = j + 1
        end
        tokenhi=tokenhi+1
        token.tokenhi = substr(template, i, j - i)  /* exact run of blanks/tabs/etc */
        i = j
        iterate
     end

    /* quoted literal => single token, quotes included; supports doubled quotes */
    if ch = "'" | ch = '"' then do
       buf=flush(buf,token,tokenhi)
       q     = ch
       qtok  = q
       i     = i + 1
       closed = 0
       do while i <= L
          c = substr(template, i, 1)
          qtok = qtok || c
          if c = q then do
          /* doubled quote => literal quote inside string */
          if i < L & substr(template, i+1, 1) = q then do
             qtok = qtok || q
             i = i + 2
             iterate
          end
          closed = 1
          i = i + 1
          leave
        end
        i = i + 1
      end
      if \closed then do
        say 'Error: unmatched quote in template'
        return -1
      end
      tokenhi=tokenhi+ 1
      token.tokenhi = qtok
      iterate
    end
     /* otherwise accumulate into current unquoted token */
    buf = buf || ch
    i = i + 1
  end

  buf=flush(buf,token,tokenhi)
/* ----------------------------------------------------------------------
 * Classify tokens in token_types
 * ----------------------------------------------------------------------
 */
  do i=1 to tokenhi
     quote=substr(token.i,1,1)
     if quote = "'" | quote = '"' then do
        token.i=strip(token.i,,quote)       ## drop quotes
        token_type.i=2
    end
    else if quote = " " then  token_type.i=5     ## blank seperator
    else if verify(token.i,'0123456789')=0 then token_type.i=3
    else if (quote='+' | quote='-' ) & verify(substr(token.i,2),'0123456789+-')=0 then token_type.i=4
    else if (quote='(') then token_type.i=6
    else token_type.i  = 1
  end
  out = 0
  do i = 1 to tokenhi
     t = token_type.i
     if t = 6 then token.i=substr(token.i,2,length(token.i)-2)
     if t \= 5 then do
        out = out + 1
        token.out      = token.i
        token_type.out = t
        iterate
     end
 /* t == 5 → decide whether to keep it */

     lt = 0; rt = 0
     ix=i-1
     if i > 1        then lt = token_type.ix
     ix=i+1
     if i < tokenhi  then rt = token_type.ix
     if (lt = 1 & rt = 1) then do  /* VAR ␠ VAR ⇒ keep as word delimiter */
        out = out + 1
        token.out      = token.i
        token_type.out = 5
     end
 end

return out

flush: procedure=.string
  arg buf=.string, expose token=.string[], expose tokenhi=.int
  if buf = '' then return ''
  tokenhi=tokenhi+1
  token.tokenhi = buf
  buf = ''
return buf

/* -------------------------------------------------
 * preCleanTemplate
 *  - Preserve inner content of quoted literals exactly
 *  - Remove whitespace only if it directly neighbors a quote
 *  - No suppression around numbers, +/-, punctuation, etc.
 *  - Supports doubled quotes inside literals ('' and "")
 * ------------------------------------------------- */
preCleanTemplate: procedure=.string
  arg template=.string

  WHITESPACE = ' '||'09'x||'0D'x||'0A'x||'0B'x||'0C'x||'A0'x
  out = ''
  L = length(template)
  i = 1
  last_out_char = ''     /* track last emitted char for "prev is quote" check */
  do while i <= L
    ch = substr(template, i, 1)
    /* ---- quoted literal: copy EXACTLY as-is, including quotes ---- */
    if ch = "'" | ch = '"' then do
      q    = ch
      qtok = q
      i    = i + 1
      closed = 0
      do while i <= L
        c = substr(template, i, 1)
        qtok = qtok || c
        if c = q then do
          /* doubled quote -> keep both quotes and continue */
          if i < L & substr(template, i+1, 1) = q then do
            qtok = qtok || q
            i = i + 2
            iterate
          end
          closed = 1
          i = i + 1
          leave
        end
        i = i + 1
      end
      if \closed then return template  /* unmatched quote: leave template as-is */

      out = out || qtok
      last_out_char = q               /* closing quote is the last emitted char */
      iterate
    end
    /* ---- whitespace run ---- */
    if pos(ch, WHITESPACE) > 0 then do
      j = i + 1
      do while j <= L & pos(substr(template, j, 1), WHITESPACE) > 0
        j = j + 1
      end
      /* peek next non-WS char */
      nextch = ''
      if j <= L then nextch = substr(template, j, 1)

      /* suppress only if whitespace touches a QUOTE on either side */
      if last_out_char = "'" | last_out_char = '"' | nextch = "'" | nextch = '"' then do
        i = j
        iterate
      end

      /* otherwise keep the whitespace as-is */
      out = out || substr(template, i, j - i)
      i = j
      last_out_char = ' '             /* non-quote marker */
      iterate
    end
    /* ---- ordinary char ---- */
    out = out || ch
    i = i + 1
    last_out_char = ch
  end
  return out