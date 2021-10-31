/* rexx */

options levelb

say 'Look for LOOP OK'
/* test 1 */
/* i=0 */
/* do for 10 */
/* i=i+1 */
/* end */
/* say i */
/* if i=10 then ok=ok '!' 'Explicit Loop' */
/* else signal DiagX('Explicit Loop') */
/* if  i \= 10        then say 'failed in test          1 ' */

 /* test 2 */
/* j=0 */
/* do until i=15 */
/* i=i+1; j=j+1 */
/* end */
/* if  i \= 15        then say 'failed in test          2 ' */
/* if  j \= 15        then say 'failed in test          2 ' */

 /* test 3 */
j=0;i=0
do until i=15
i=i+1; j=j+1
end
if  i \= 15        then say 'failed in test          3 '
if  j \= 15        then say 'failed in test          3 '

/* test 4 */
j=0;i=0
do while i<12
i=i+1; j=j+1
end
if  i \= 12        then say 'failed in test          4 '
if  j \= 12        then say 'failed in test          4 '


/* test 5 */
j=0;i=0
do while i<12
  i=i+1
  if i = 11 then leave
  j=j+1
end
if  i \= 11        then say 'failed in test          5 '
if  j \= 10        then say 'failed in test          5 '

/* test 6 */
  j=0; flag='1'; fail=''
  do i  =  1 to 12 by 2 while(i<=7)
    j=j+1; end
if  i \= 9        then say 'failed in test          5 '
if  j \= 4        then say 'failed in test          5 '

/* test 7 */
  j=0;i=0
  do i=0-1 to(10-22)by 20-22 until i<-7
    j=j+1; end
if  i \= -9        then say 'failed in test          7 '
if  j \= 5        then say 'failed in test          7 '

say 'Loop OK'
