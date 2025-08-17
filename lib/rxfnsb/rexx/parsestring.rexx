/* rexx */
options levelb

namespace rxfnsb expose parseString

/* ----------------------------------------------------------------------
 * Parse the string, using the found tokens and types
 *   parse_string      – the source to parse
 *   token.            – compiled template tokens (literal content for type=2)
 *   token_type.       – 1=VAR, 2=LIT, 3=ABS POS, 4=REL POS, 5=UNQUOTED BLANK
 * Outputs:
 *   variable.         – variable names in template order (for type=1)
 *   variable_content. – matched field values aligned with variable.
 * ----------------------------------------------------------------------
 */
parsestring: procedure
  arg parse_string=.string, tokenhi=.int, token=.string[],token_type=.string[],expose variable=.string[],expose variable_content=.string[]

  /* treat these as whitespace for type=5 */
  WHITESPACE = ' '||'09'x||'0D'x||'0A'x||'0B'x||'0C'x||'A0'x

  pointer       = 1                 /* 1-based index into parse_string   */
  j             = 0                 /* count of variables assigned       */
  lastVarIndex  = 0                 /* k index of the last VAR token     */
  L             = length(parse_string)
  do k = 1 to tokenhi
      type = token_type.k
      if type = 1 then do                 /* VAR name */
         j = j + 1
         variable.j = token.k
         lastVarIndex = k
      end
      else if type = 3 | type = 4 then do       /* ABS/REL position */
        newpos = token.k
        if type = 4 then newpos = pointer + newpos
        if newpos < 1 then newpos = 1
        if newpos > L + 1 then newpos = L + 1

        if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
        /* slice up to (but not including) newpos */
           if newpos > pointer then variable_content.j = substr(parse_string, pointer, newpos - pointer)
           else variable_content.j = ''        /* moving back or same column → empty */
           lastVarIndex = 0
        end
        pointer = newpos
      end
      else if type = 2 then do                 /* literal delimiter (token.k is literal text) */
        chunk = substr(parse_string, pointer)
        p = pos(token.k, chunk)
        if p = 0 then do                   /* not found: remainder goes to pending var */
           if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
              variable_content.j = chunk
              lastVarIndex = 0
           end
           pointer = L + 1
        end
        else do
           if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
              variable_content.j = substr(parse_string, pointer, p - 1)
              lastVarIndex = 0
           end
           pointer = pointer + p - 1 + length(token.k)   /* hop past delimiter */
        end
      end
      else if type = 5 then do                 /* unquoted blank: any whitespace run */
     /* find first whitespace from current pointer */
        chunk = substr(parse_string, pointer)
        lenC  = length(chunk)
        p = 0
        do xi = 1 to lenC
           if pos(substr(chunk, xi, 1), WHITESPACE) = 0 then iterate
           p = xi
           leave
        end
        if p = 0 then do                   /* no whitespace found: remainder to var */
           if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
              variable_content.j = chunk
              lastVarIndex = 0
           end
           pointer = L + 1
        end
        else do
           if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
              variable_content.j = substr(parse_string, pointer, p - 1)
              lastVarIndex = 0
           end
        /* skip the entire whitespace run (treat as a single blank) */
           runEnd = p
           do while runEnd <= lenC & pos(substr(chunk, runEnd, 1), WHITESPACE) > 0
              runEnd = runEnd + 1
           end
           pointer = pointer + runEnd - 1
        end
      end
  end

  /* final fallback: remaining tail to the last pending variable */
  if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
     if pointer <= L then variable_content.j = substr(parse_string, pointer)
     else variable_content.j = ''
     lastVarIndex = 0
  end
return