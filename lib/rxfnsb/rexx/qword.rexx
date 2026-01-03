/* rexx */
options levelb

namespace rxfnsb expose qword

/* ------------------------------------------------------------------
 * QWORD(line, wanted)
 *
 *   Returns the Nth word from 'line', keeping quoted strings intact.
 *
 *   Optimised version using assembler helper instructions:
 *     FNDBLNK   – find next blank or tab
 *     FNDNBLNK  – find next non-blank or non-tab
 *     STRPOS    – find next occurrence of a character (1-based)
 *     STRCHAR   – get character value (as byte) at position (0-based)
 *     STRLEN    – length of string
 *
 *   Note:
 *     STRPOS   → 1-based result (REXX compatible)
 *     FNDBLNK  → 0-based result (<0 if not found)
 *     FNDNBLNK → 0-based result (<0 if not found)
 *
 *   The 1-based / 0-based asymmetry is deliberate:
 *   it reduces conversion overhead in the most common paths.
 * ------------------------------------------------------------------ */
qword: procedure = .string
  arg line = .string, wanted = .int

  /* --- Quick validity checks --- */
  if wanted < 1 then return ''                     /* invalid word index */
  len = .int
  assembler strlen len, line                       /* total string length */
  if len = 0 then return ''                        /* empty string */

  /* --- Early-out optimisation: no quotes, use plain WORD() --- */
  n1 = .int
  n2 = .int
  assembler FNDNBLNK n1, line, n1                  /* find first non-blank */
  if n1 < 0 then return ''                         /* line contains only blanks */

  /* Look for either double or single quotes in the string. */
  qch = '"'
  n1=1
  assembler strpos n1, qch, line
  qch = "'"
  n2=1
  assembler strpos n2, qch, line

  /* If neither quote exists, we can use native WORD() directly.
     This branch avoids all scanning and assembler overhead. */
  if n1 + n2 = 0 then return word(line, wanted)

  /* --- Initialise loop state --- */
  nbpos = .int             /* position of next non-blank (0-based) */
  bpos  = .int             /* position of next blank (0-based) */
  pos   = 0                /* current position in text (0-based) */
  wordno = 0               /* current word number */
  ch   = .int              /* numeric code of current character */
  clch = .int              /* numeric code of next character (lookahead for quotes) */

  /* Predefine ASCII quote characters (fast lookup, no comparison strings) */
  quote[34] = '"'          /* double quote (ASCII 34) */
  quote[39] = "'"          /* single quote (ASCII 39) */

  /* --- Main parsing loop --- */
  do while pos < len
     /* Find the next non-blank/tab → start of next token. */
     assembler fndnblnk nbpos, line, pos
     if nbpos < 0 then leave                         /* no more words */
     i = nbpos + 1                                   /* convert to 1-based index */
     start = i                                       /* remember start of token */

     /* Fetch the first character of this token directly as byte code.
        STRCHAR avoids allocating a substring, faster than SUBSTR(). */
     j = i - 1                                       /* convert to 0-based offset */
     assembler strchar ch, line, j                   /* read character as byte value */

     /* --- Handle quoted strings --- */
     if ch = 34 | ch = 39 then do                    /* if it’s a quote: " or ' */
        qch = quote[ch]                              /* store the quote type from table */
        n = i + 1                                    /* position after opening quote */

        /* --- Search for closing quote (handle doubled quotes like "") --- */
        do forever
           assembler strpos n, qch, line             /* find next occurrence of same quote */
           if n = 0 then do                          /* unmatched quote → treat rest as quoted */
              n = len + 1                            /* simulate "closing" at end of line */
              leave
           end

           /* Peek ahead: if we see another identical quote immediately after,
              it's a doubled quote (escaped quote) → skip both and continue. */
           assembler strchar clch, line, n            /* get character after quote */
           if n < len & clch = ch then do
              n = n + 2                               /* skip over escaped quote */
              iterate                                 /* continue scanning inside string */
           end

           /* Otherwise: found a proper closing quote, stop searching. */
           leave
        end

        /* Count and possibly return this quoted word. */
        wordno = wordno + 1
        if wordno = wanted then return substr(line, i, n - i + 1)
        /* Continue scanning after the quoted region. */
        pos = n
        iterate
     end

     /* --- Handle normal (non-quoted) word --- */
     assembler fndblnk bpos, line, nbpos             /* find next blank after this word */
     if bpos < 0 then bpos = len                     /* no more blanks → end of string */
     wordno = wordno + 1
     if wordno = wanted then do
        return strip(substr(line, start, bpos - nbpos))
     end
     /* Move to next scanning position (after word). */
     pos = bpos + 1
  end

return ''                                            /* wanted word not found */
