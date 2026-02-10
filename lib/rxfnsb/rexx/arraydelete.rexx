/* rexx */
options levelb

namespace rxfnsb expose arrayDelete
/* ----------------------------------------------------------------------
 * ARRAYDELETE(array, from, count)
 *
 * Closes a gap of COUNT elements in ARRAY starting at position FROM.
 *
 * Elements ARRAY[FROM] .. ARRAY[FROM+COUNT-1] are removed.
 * Elements above that range are shifted downward to close the gap.
 *
 * Convention:
 *   - array[0] holds the number of elements
 *   - elements are in array[1] .. array[array[0]]
 *
 * Returns:
 *   The new number of elements in ARRAY
 * ----------------------------------------------------------------------
 */
arraydelete: procedure=.int
  arg expose array=.string[], from=1, count=0

  hi = array[0]
  if hi < 1 then return hi
  if count < 1 then return hi
  if from  < 1 then return hi
  if from  > hi then return hi

  /* clamp delete length to available elements */
  if from + count - 1 > hi then count = hi - from + 1

  /* shift down to close the gap */
  do i = from + count to hi
     array[i - count] = array[i]
  end

  /* clear trailing slots (optional but nice hygiene) dropped for the moment
  do i = hi - count + 1 to hi
     array[i] = ''
  end
  */
  hi=hi-count
  assembler SETATTRS array,hi
return array[0]
