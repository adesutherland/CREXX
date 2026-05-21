/* arraysort.crx
   ARRAYSORT sorts the elements of an array in ascending order.
   The function returns the number of elements in the sorted array.
*/
options levelb
import rxfnsb

array = .string[]

say 'Before Sort'
say '-----------'

/* Fill the array in reverse order */
do i = 1000 to 1 by -1
   array[i] = 'Record 'right(i, 6)
   say right(i, 6) array[i]
end

/* Sort the array alphabetically */
sorted = arraysort(array)

say
say 'After Sort'
say '----------'

/* Display the sorted result */
do i = 1 to sorted
   say right(i, 6) array[i]
end