/* GETPI Plugin Test */
options levelb
import arrays
import rxfnsb

imax=100000
## Step 1 Sort Array descending order, sort criteria is column 6
j=imax
do i=1 to imax
   array.i = 'Record 'right(i,8,'0')
   j=j-1
end
## Step 1a Output initial array
call list_array array,1,10
call list_array array,imax-10,imax

## Step 2 Perform Sort
say '*** sort array at position 14,DESCENDING'
say time('l')
##                    x -------- sort at position 14
##                    !   x -------- sort order descending
call shell_sort array,14,'DESC'
say time('l')
## Step 3 output sorted array
say '*** Sort Result'
call list_array array,1,10
call list_array array,imax-10,imax

## Step 3 Sort Array ascending order, sort criteria is column 9
j=imax
do i=1 to imax
   array.i = 'X'right(j,8,'0')
   j=j-1
end
## Step 4 Output initial array
call list_array array,1,10
call list_array array,imax-10,imax## Step 2 Perform Sort
say time('l')
##                    x -------- sort at position 6
##                    !   x -------- sort order ascending
say '*** sort array at position 6, ASCENDING'
call shell_sort array,6,'ASC'
say time('l')
## Step 4a output sorted array
call list_array array,1,10
call list_array array,imax-10,imax## Step 2 Perform Sort

say '*** Sort Result'
## Step 5 Reverse array order
say '*** Reverse array'
say time('l')
call reverse_array array
say time('l')
say '*** Reversed array'
call list_array array,1,10
call list_array array,imax-10,imax## Step 2 Perform Sort

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
call list_array array,1,10
call list_array array,imax-10,imax## Step 2 Perform Sort

## Step 8 insert new entries
say '*** add new array entries (in the middle)'
say insert_array(array,3,2)     ## add empty line 3 and 4
array.3='new entry 3'
array.4='new entry 4'
call list_array array,1,10
call list_array array,imax-10,imax

## Step 9 copy entire array
say '*** copy entire array'
newarray[1]=''   ## basic array definition for new array is required
say copy_array(array,newarray,11,11)
call list_array newarray,1,10
call list_array newarray,imax-10,imax

exit


