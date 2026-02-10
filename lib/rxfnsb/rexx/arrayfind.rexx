options levelb
namespace rxfnsb expose arrayfind

/* --------------------------------------------------------------------------------------
 * arrayfind(find, array[, from[, case]])
 *
 * Searches an array of strings for the first element containing a substring.
 *
 * Arguments:
 *   find   - substring to search for
 *   array  - array of strings (array[0] = element count)
 *   from   - starting array position (default: 1)
 *   case   - case sensitivity flag:
 *            1 = case-sensitive (default)
 *            0 = case-insensitive
 *
 * Returns:
 *   Index (1-based) of the first matching array element,
 *   or 0 if no match is found.
 * --------------------------------------------------------------------------------------
 */
arrayfind: procedure=.int
  arg find=.string,array=.string[],from=1,case=1
  if from < 1 then from = 1
  if from > array[0] then return 0

  if case=1 then do i=from to array[0]     /* case sensitive search */
     if pos(find,array[i])>0 then return i
  end
  else do       /* case insensitive search */
      ufind=upper(find)
      do i=from to array[0]
         if pos(ufind,upper(array[i]))>0 then return i
       end
  end
return 0