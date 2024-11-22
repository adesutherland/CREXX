/* GETPI Plugin Test */
options levelb
import arrays
import rxfnsb

imax=25
## Step 1 Sort Array descending order, sort criteria is column 6
j=imax
do i=1 to imax
   array.i = 'Record 'i
   j=j-1
end
## Step 1a Output initial array
do i=1 to 10 ;         say i array.i ; end
do i=imax-10 to imax ; say i array.i ; end
## Step 2 Perform Sort
say time('l')
##                    x -------- sort at position 6
##                    !   x -------- sort order descending
call shell_sort array,6,'DESC'
say time('l')
## Step 3 output sorted array
say '*** Sort Result'
do i=1 to 10 ;         say i array.i ; end
do i=imax-10 to imax ; say i array.i ; end

## Step 4 Sort Array ascending order, sort criteria is column 5
j=imax
do i=1 to imax
   array.i = 'X'right(j,8,'0')
   j=j-1
end
## Step 1a Output initial array
do i=1 to 10 ;         say i array.i ; end
do i=imax-10 to imax ; say i array.i ; end
## Step 2 Perform Sort
say time('l')
##                    x -------- sort at position 5
##                    !   x -------- sort order ascending
call shell_sort array,6,'ASC'
say time('l')
## Step 3 output sorted array
say '*** Sort Result'
do i=1 to 10 ;         say i array.i ; end
do i=imax-10 to imax ; say i array.i ; end
## Step 5 Reverse array order
say time('l')
call reverse_array array
say time('l')
say '*** Reversed array'
do i=1 to 10 ;         say i array.i ; end
do i=imax-10 to imax ; say i array.i ; end
## Step 6 search string in array
say '*** Search array'
k=search_array(array,'5',1)
say 'found at 'k array.k
k=search_array(array,'11',k+1)
say 'next found at 'k array.k
## Step 7 remove array entries
say '*** remove array entries'
say 'Lines removed 'delete_array(array,15,99)
say 'New array hi 'array.0
do i=1 to array.0 ; say i 'Delete 'array.i ; end
## Step 8 insert new entries
say '*** add new array entries (in the middle)'
say insert_array(array,3,2)     ## add empty line 3 and 4
array.3='new entry 3'
array.4='new entry 4'
do i=1 to array.0 ; say i 'insert 'array.i ; end
## Step 9 copy entire array
say '*** copy entire array'
newarray[1]=''   ## basic array definition for new array is required
say copy_array(array,newarray,11,11)
do i=1 to newarray.0 ; say i 'new array 'newarray.i ; end

exit


