/* rexx */
options levelb

namespace rxfnsb expose qextractall

/* ------------------------------------------------------------------
 *  Function:  QEXTRACTALL
 *  Purpose:   Extracts all top-level substrings enclosed by matching
 *             delimiters (e.g. (), {}, BEGIN/END, <!--/-->), ignoring
 *             quoted text and nested inner pairs.
 *
 *  Usage:     parts. = QEXTRACTALL(open, close, text [, start])
 *
 *   Arguments:
 *     open   - Opening delimiter (string, one or more characters)
 *     close  - Matching closing delimiter (string, one or more chars)
 *     text   - The string to scan
 *     start  - (optional) Position to begin searching. Default = 1.
 *
 *  Returns:
 *     A stem array:
 *        parts.0 - number of segments found
 *        parts.1 - first complete segment
 *        parts.2 - second segment, etc.
 *     Returns .0=0 if no complete pair is found.
 *
 *  Description:
 *     Repeatedly calls QEXTRACTPAIR() to locate and extract each
 *     top-level delimited segment, skipping any text or partial
 *     fragments between them. Handles nested pairs and quoted text.
 *
 *  Example:
 *     s = 'BEGIN one BEGIN two END END BEGIN three END'
 *     segs. = qextractall('BEGIN', 'END', s)
 *     do i = 1 to segs.0
 *        say i':' segs.i
 *     end
 *
 *  Output:
 *     1: BEGIN one BEGIN two END END
 *     2: BEGIN three END
 *
 *  Dependencies:
 *     Requires QEXTRACTPAIR() and QPOS().
 *
 *  Notes:
 *     - Multi-character delimiters are fully supported.
 *     - Ignores delimiters appearing inside quoted strings.
 *     - Only top-level (non-nested) segments are returned.
 *  ------------------------------------------------------------------
 */
qextractall: procedure=.string[]
   arg open=.string, close=.string, text=.string, mode='X'

   parts = .string[]
   count  = 0
   pos    = 1
   lenOpen = length(open)
   lenText = length(text)

   do while pos <= lenText
      /* find next opening delimiter */
      firstPos = qpos(open, text, pos)
      if firstPos = 0 then leave

      /* extract full segment */
      seg = qextractpair(open, close, text, firstPos,mode)
      if seg = '' then leave

      count = count + 1
      parts.count = seg

      /* move beyond this segment */
      pos = firstPos + length(seg)
   end
return parts