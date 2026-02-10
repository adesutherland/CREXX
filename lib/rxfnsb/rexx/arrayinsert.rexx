/* rexx */
options levelb

namespace rxfnsb expose arrayinsert
/* ----------------------------------------------------------------------
 * ARRAYINSERT(array, from, count[, default])
 *
 * Opens a gap of COUNT elements in ARRAY at position FROM.
 *
 * Elements at ARRAY[FROM] and above are shifted upward to make room.
 * The newly created slots ARRAY[FROM] .. ARRAY[FROM+COUNT-1]
 * are initialised to DEFAULT (default "").
 *
 * Convention:
 *   - array[0] holds the number of elements
 *   - elements are in array[1] .. array[array[0]]
 *
 * Returns:
 *   The new number of elements in ARRAY
 * ----------------------------------------------------------------------
 */
arrayinsert: procedure=.int
  arg expose array=.string[], from=1, count=0, default=""

  hi = array[0]
  if count < 1 then return hi
  if from < 1 then return hi

  /* clamp: inserting beyond end means append at end+1 */
  if from > hi + 1 then from = hi + 1

  /* shift up to make room (work backwards to avoid overwriting) */
  do i = hi to from by -1
    array[i + count] = array[i]
  end

  /* create the empty slots */
  do i = from to from + count - 1
    array[i] = default
  end

return array[0]