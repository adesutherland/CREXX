/* rexx */
options levelb

namespace rxfnsb expose arraysort

/* ----------------------------------------------------------------------
 * ArraySort
 *
 * Shell sort for a CREXX string array.
 *
 * The array is sorted lexically (case-sensitive) using
 *     substr(element, offset)
 * as the comparison key, allowing sorting on a substring within
 * each element (e.g. fixed-format records).
 *
 * ARRAY CONVENTION
 *   - a[0] holds the element count (maintained automatically)
 *   - elements are stored in a[1] .. a[a[0]]
 *
 * PARAMETERS
 *   a      : .string[]  Array to be sorted
 *   offset : integer   1-based character position for the sort key (default 1)
 *   order  : string    'ASC' (default) or 'DESC'
 *
 * IMPLEMENTATION NOTES
 *   - Classic Shell sort with gap halving
 *   - Gapped insertion sort per pass
 *   - Integer division for gap calculation is enforced via IDIV
 *     to avoid dependency on numeric mode semantics
 *
 * RETURNS
 *   The sorted array a[]
 * ----------------------------------------------------------------------
 */
arraysort: procedure=.string[]
  arg a=.string[], offset=1, order='ASC'

  n = a[0]
  if n < 2 then return a
  if offset < 1 then offset = 1

  order = upper(order)
  asc = (substr(order,1,3) = 'ASC')
  gap = n
  assembler idiv gap, gap, 2

  do while gap > 0
     do i = gap + 1 to n
        temp = a[i]
        key  = substr(temp, offset)
        j = i
        if asc then do
           do while j > gap
              prev = a[j-gap]
              if substr(prev, offset) <= key then leave
              a[j] = prev
              j = j - gap
           end
        end
        else do
           do while j > gap
              prev = a[j-gap]
              if substr(prev, offset) >= key then leave
              a[j] = prev
              j = j - gap
           end
        end
        a[j] = temp
     end
     assembler idiv gap, gap, 2
  end
  return a