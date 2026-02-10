/* rexx */
options levelb

namespace rxfnsb expose arraycopy
/* ------------------------------------------------------------------
 *  arraycopy
 *
 *  Copy a slice of a Rexx array (1-based, size in a[0]).
 *
 *  Semantics:
 *    - from  : start position (1-based)
 *              from < 0 counts from the end (-1 = last element)
 *    - count : number of elements to copy
 *              count <= 0 means "copy to end"
 *
 *  Notes:
 *    - Negative FROM only affects the start position.
 *    - COPY direction is always forward (toward higher indices).
 *    - "copy to end" always means up to a[n], never wrap-around.
 *
 *  Behavior:
 *    - Indices are clamped to array bounds
 *    - Out-of-range start results in an empty array
 *    - No errors are raised for invalid ranges
 *
 *  Examples:
 *    arraycopy(a)            -> full array
 *    arraycopy(a, 3)         -> elements 3 .. end
 *    arraycopy(a, 3, 2)      -> elements 3 .. 4
 *    arraycopy(a, -1)        -> last element
 *    arraycopy(a, -3)        -> last 3 elements
 *    arraycopy(a, -3, 2)     -> elements n-2 .. n-1
 *
 * ------------------------------------------------------------------ */
arraycopy: procedure=.string[]
  arg a=.string[], from=1, count=0

  n = a[0]
  b = .string[]
/* map negative from to 1-based index */
  if from < 0 then from = n + from + 1

  /* out-of-range start -> empty */
  if from < 1 | from > n then return b

  /* count <= 0 => copy to end */
  if count <= 0 then count = n - from + 1

  /* clamp count to available elements */
  if count > (n - from + 1) then count = n - from + 1

  last = from + count - 1
  j = 0
  do i = from to last
     j = j + 1
     b[j] = a[i]
  end
return b