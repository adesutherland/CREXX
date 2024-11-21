/* GETPI Plugin Test */
options levelb
import arrays
import rxfnsb

imax=100000
## Step 1 Sort Array descending order, sort criteria is column 6
j=imax
do i=1 to imax
   array.i = 'A'right(j,8,'0')
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
k=search_array(array,'11',1)
say k array.k
k=search_array(array,'11',k+1)
say k array.k
exit


