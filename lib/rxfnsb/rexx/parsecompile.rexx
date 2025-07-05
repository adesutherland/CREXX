/* rexx */
options levelb

namespace rxfnsb expose parseCompile

/* ----------------------------------------------------------------------
 * Compile the given template, split it in tokens and token.ktypes
 * ----------------------------------------------------------------------
 */
parseCompile: Procedure=.int
  arg template=.string,expose token=.string[],expose token_type=.string[]
  k = 0
  do i = 1 to 4096  /* Large upper bound */
     wrd = word(template, i)
     if wrd = '' then leave   /* No more words */
     do while wrd \= ''
        p1 = pos("'", wrd)
        p2 = pos('"', wrd)
        if p1+p2=0 then do
           k = k + 1
           token.k = wrd
           wrd = ''     /* Done with this word */
           leave
        end
        if p2 = 0 | (p1>0 & p1<p2) then quote = "'"
        else do
           p1=p2
           quote = '"'   /* either " exists before ' or only " exists */
        end
        if p1 > 1 then do     /* Add left hand side before quote, if any */
           k = k + 1
           token.k = substr(wrd, 1, p1 - 1)
        end
     /* Find matching closing quote */
        p2 = pos(quote, wrd, p1 + 1)
        if p2 = 0 then do
           say 'Error: unmatched quote in word:' wrd
           leave
        end
      /* Extract quoted segment */
        k = k + 1
        plen=p2 - p1+1
        token.k = substr(wrd, p1,plen )
        if p2>=length(wrd) then wrd=''   /* Cut off processed part */
        else wrd = substr(wrd, p2 + 1)
     end
  end
/* ----------------------------------------------------------------------
 * Classify tokens in token_types
 * ----------------------------------------------------------------------
 */
  do i=1 to token.0
     quote=substr(token.i,1,1)
     if quote = "'" | quote = '"' then do
        token.i=strip(token.i,,quote)       ## drop quotes
        token_type.i=2
    end
    else if verify(token.i,'0123456789')=0 then token_type.i=3
    else if (quote='+' | quote='-' ) & verify(substr(token.i,2),'0123456789+-')=0 then token_type.i=4
    else token_type.i  = 1
  end
return token.0