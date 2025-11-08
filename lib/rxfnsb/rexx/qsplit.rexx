/* rexx */
options levelb

namespace rxfnsb expose qsplit

/* ------------------------------------------------------------------
 *  QSPLIT(text, sep)
 *  Splits 'text' into parts by 'sep', ignoring separators inside
 *  quotes (handles '' and "" escaped quotes).
 *  Returns a stem array .parts.1, .parts.2, ...  and .0 = count.
 * ------------------------------------------------------------------
 */
qsplit: procedure=.string[]
   arg text=.string, sep=.string
   count  = 0
   start  = 1
   parts=.string[]
   do forever
      npos = qpos(sep, text, start)
      if npos = 0 then do
         count = count + 1
         parts[count] = substr(text, start)
         leave
      end
      count = count + 1
      parts[count] = strip(substr(text, start, npos - start))
      start = npos + length(sep)
   end
return parts