/* rexx */
options levelb

namespace rxfnsb expose qextractpair

/* ------------------------------------------------------------------
 *  Function:  QEXTRACTPAIR
 *  Purpose:   Returns the full substring enclosed by a balanced pair
 *             of delimiters (e.g. (), {}, []), skipping any that
 *             occur inside quoted text and supporting nested pairs.
 *
 *  Usage:     segment = QEXTRACTPAIR(open, close, text [, start])
 *
 *  Arguments:
 *     open   - Opening delimiter (e.g. "(" or "{")
 *     close  - Matching closing delimiter.
 *     text   - Source string to scan.
 *     start  - (optional) Position in 'text' at which to start
 *              searching for the first 'open' character.
 *              Default = 1.
 *
 *  Returns:
 *     The substring beginning with the first opening delimiter found
 *     at or after 'start' and ending with its matching closing
 *     delimiter. Returns '' if no complete pair is found.
 *
 *  Description:
 *     QEXTRACTPAIR uses QPOS() to locate unquoted occurrences of
 *     'open' and 'close'.  It increments depth for each nested 'open'
 *     and decrements for each 'close'. When depth returns to zero,
 *     the entire segment is returned, including both delimiters.
 *
 *  Example:
 *     s = 'func(a, (b, "c(d)"), e)'
 *     say qextractpair('(', ')', s)
 *        -> (a, (b, "c(d)"), e)
 *
 *     s = 'outer { inner { "ignore { this }" } tail }'
 *     say qextractpair('{', '}', s)
 *        -> { inner { "ignore { this }" } tail }
 *
 *  Dependencies:
 *     Requires QPOS() for quote-aware scanning.
 *
 *  Notes:
 *     - Ignores delimiters inside quoted strings.
 *     - Handles nested pairs correctly.
 *     - Behavior is case-sensitive, matching REXX semantics.
 *  ------------------------------------------------------------------
 */
qextractpair: procedure=.string
   arg open=.string, close=.string, text=.string, start=1, mode='x'

   mode=upper(left(mode,1))
   if mode='E' then mode='X'
   if mode='I' then nop
   else if mode='X' then nop
   else mode='X'                    ## default value is X for exclusive
   lenOpen  = length(open)
   lenClose = length(close)

   /* find the first opening delimiter after start */
   firstPos = qpos(open, text, start)
   if firstPos = 0 then return ''

   depth = 1
   pos = firstPos + lenOpen

   do forever
      nextOpen  = qpos(open,  text, pos)
      nextClose = qpos(close, text, pos)

      if nextOpen = 0 & nextClose = 0 then return ''

      if nextClose = 0 | (nextOpen > 0 & nextOpen < nextClose) then do
         depth = depth + 1
         pos = nextOpen + lenOpen
      end
      else do
         depth = depth - 1
         pos = nextClose + lenClose
         if depth = 0 then do
            if mode = 'X' then return substr(text, firstPos + length(open), nextClose - firstPos - length(open))
            else return substr(text, firstPos, nextClose - firstPos + length(close))
         end

       end
   end
return ""