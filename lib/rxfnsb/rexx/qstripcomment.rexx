/* rexx */
options levelb

namespace rxfnsb expose qstripcomment

/* ------------------------------------------------------------------
 *   Function:  QCOMMENT
 *  Purpose:   Remove comment segments from a string.
 *
 *  Usage:     clean = QCOMMENT(open, close, text)
 *
 *  Arguments:
 *     open   - Opening delimiter (e.g. '/*' or '--')
 *     close  - (optional) Closing delimiter (e.g. '*/')
 *              If omitted or empty, the comment ends at line end.
 *     text   - Input string to clean.
 *
 *  Returns:
 *     The input text with all comment segments removed.
 *
 *  Notes:
 *     - Quote-safe: ignores comment markers inside quoted strings.
 *     - Handles multiple comments per line.
 *     - Handles unterminated block comments gracefully.
 *  ------------------------------------------------------------------
 */
qstripcomment: procedure=.string
  arg open=.string, close='', text=.string
  if close = '' then xclose = 0          /* 0 means line comment mode */
  else xclose=1

  out = ''
  pos = 1
  len = length(text)

  do while pos <= len
     cpos = qpos(open, text, pos)
     if cpos = 0 then do
        out = out || substr(text, pos)
        leave
     end

    /* copy everything before the comment */
    out = out || substr(text, pos, cpos - pos)

    if xclose = 0 then do      /* line comment: skip to end of line */
       epos = pos('0D0A'x, text, cpos)
       if epos = 0 then epos = len + 1
       pos = epos
    end
    else do    /* block comment: extract whole segment (inclusive) */
       seg = qextractpair(open, close, text, cpos, 'inclusive')
       if seg = '' then leave   /* no closing delimiter → cut rest */
       pos = cpos + length(seg)
    end
  end
return out