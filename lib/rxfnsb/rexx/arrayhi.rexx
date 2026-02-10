/* rexx */
options levelb

namespace rxfnsb expose arrayhi

/* ------------------------------------------------------------------
 * ARRAYHI(array[, 'GET' | 'SET'[, new_hi]])
 *
 * Get or set the high index (size) of a Rexx array.
 *
 * CALL SYNTAX
 *   hi = ARRAYHI(array)
 *   hi = ARRAYHI(array, 'GET')
 *   hi = ARRAYHI(array, 'SET', new_hi)
 *
 * SEMANTICS
 *   - Default mode is GET: returns array[0]
 *   - In SET mode, the array may only be shrunk
 *
 * BEHAVIOR (SET mode)
 *   - If new_hi >= current array size, the request is ignored
 *   - If new_hi < 1, the request is ignored
 *   - If 1 <= new_hi < current array size:
 *       * elements above new_hi are discarded
 *       * array[0] is updated accordingly
 *
 * CONVENTION
 *   - array[0] holds the number of elements
 *   - elements are stored in array[1] .. array[array[0]]
 *
 * NOTES
 *   - The array is modified in place in SET mode
 *   - ARRAYHI never grows an array
 *   - No new elements are allocated or initialised
 *
 * RETURNS
 *   - The current high index after the operation
 *
 * ------------------------------------------------------------------ */
arrayhi: procedure=.int
  arg array=.string[],mode='GET',newhi=0
  mode=upper(mode)
  if mode='SET' then do
     if newhi>=array[0] then return array[0]
     if newhi< 1 then return array[0]
     assembler SETATTRS array,newhi
  end
return array[0]