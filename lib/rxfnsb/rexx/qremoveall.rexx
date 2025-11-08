/* rexx */
options levelb

namespace rxfnsb expose qremoveall

/* ------------------------------------------------------------------
 *  Function:  QREMOVEALL
 *  Purpose:   Removes all delimited segments from a string,
 *             respecting quotes and nested pairs.
 *
 *  Usage:     clean = QREMOVEALL(open, close, text [, mode])
 *
 *  Arguments:
 *     open   - Opening delimiter (string, one or more characters)
 *     close  - Matching closing delimiter
 *     text   - Source string to clean
 *     mode   - (optional) 'exclusive' or 'inclusive'
 *              'inclusive' (default) removes including delimiters
 *              'exclusive' removes only the inner content
 *
 *  Returns:
 *     A copy of the text with all delimited blocks removed.
 *
 *  Notes:
 *     - Quote-safe and nesting-aware (via QEXTRACTALL)
 *     - Default behavior removes full matched segments
 *     - For exclusive mode, keeps delimiters but removes inner content
 *     - Multiple nested or sequential blocks supported
 *  ------------------------------------------------------------------
 */
qremoveall: procedure=.string
  arg open=.string, close=.string, text=.string, mode='I'

  out = text
  blocks = qextractall(open, close, text, mode)

  do i = 1 to blocks.0
     out = changestr(blocks.i, out, '')
  end
return out