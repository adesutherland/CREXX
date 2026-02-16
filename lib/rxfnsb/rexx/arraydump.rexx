/* rexx */
options levelb

namespace rxfnsb expose arraydump

/* ----------------------------------------------------------------------
 * ARRAYDUMP(array[, from[, to[, flags[, hdr[, prefix]]]]])
 *
 * flags:
 *   I = print index
 *   C = print total count in header
 *   L = print element length
 *   Q = quote elements with '...'
 *
 *   E = show <empty> for empty strings
 *   T = show raw -> strip(raw) (both quoted)
 *   N = make control/non-printable characters visible as \xNN
 *
 * Returns: number of printed elements
 * ----------------------------------------------------------------------
 */
arraydump: procedure=.int
  arg array=.string[], from=1, tto=0, flags="IC", hdr="", prefix=""

  n = array[0]
  if from < 1 then from = 1
  if tto <= 0 | tto > n then tto = n
  if from > tto then return 0

  uflags = upper(flags)

  showI = pos("I", uflags) > 0
  showC = pos("C", uflags) > 0
  showL = pos("L", uflags) > 0
  quote = pos("Q", uflags) > 0

  showE = pos("E", uflags) > 0
  showT = pos("T", uflags) > 0
  showN = pos("N", uflags) > 0

  if hdr = "" then hdr = "Array dump"
  if showC then say prefix||hdr||" (count="n", range="from"-"tto"), flags="uflags
  else          say prefix||hdr||" (range="from"-"tto"), flags="uflags

  printed = 0
  do i = from to tto
     v = array[i]

  /* N: make non-printables visible */
     if showN then v = _vis_np(v)
     out = ""
     if showI then out = out || right(i, 6,'0')||" "
     if showL then out = out || "len="right(length(v),5)||" "
     if showT then do
        raw = v
        trimmed = strip(v)
        out = out || _squote(raw) || " -> " || _squote(trimmed)
     end
     else do
        if v = "" & showE then out = out || "<empty>"
        else if quote then out = out || _squote(v)
        else out = out || v
     end
     say prefix||out
     printed=printed+1
  end
return printed


/******** helpers ********/

/* single-quote a string; doubles embedded single quotes */
_squote: procedure=.string
  arg s=.string
  if pos("'", s) =1 then return '"' || s || '"'
return "'" || s || "'"

/* replace non-printable ASCII control chars with \xNN
 * (keeps normal printable bytes as-is)
 */
_vis_np: procedure=.string
  arg s=.string
  out = ""
  do k = 1 to length(s)
     ch = substr(s, k, 1)
     c  = c2d(ch)
  /* treat ASCII control chars + DEL as non-printable */
     if c < 32 | c = 127 then do
        out = out || "\x" || d2x(c, 2)
     end
     else out = out || ch
  end
return out
