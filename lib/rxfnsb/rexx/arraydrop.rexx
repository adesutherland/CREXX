/* rexx */
options levelb

namespace rxfnsb expose arraydrop

/* ------------------------------------------------------------------
 *  arraydrop
 *
 *  Remove all elements from a Rexx array.
 *
 *  Behavior:
 *    - Clears the array in place by resetting its size to zero
 *    - All existing elements become inaccessible
 *    - The array variable itself remains valid
 *
 *  Notes:
 *    - This function modifies the passed array directly
 *    - Intended for arrays using a[0] as element count
 *
 *  Returns:
 *    0 on success
 *
 * ------------------------------------------------------------------ */

arraydrop: procedure=.int
  arg array=.string[]
  assembler SETATTRS array,0
return 0