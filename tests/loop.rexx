/* rexx */

options levelb

say 'Look for LOOP OK'
/* test 1 */
i=0
do 10
i=i+1
end
if  i \= 10        then say 'failed in test          1 '

 /* test 2 */
j=0
do forever
  i=i+1; j=j+1
  if j=100 then leave
end
if  j \= 100 then say 'failed in test          2 '

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
if  i \= 9         then say 'failed in test          6 '
if  j \= 4         then say 'failed in test          6 '

/* test 7 */
  j=0;i=0
  do i=0-1 to(10-22)by 20-22 until i<-7
    j=j+1; end
if  i \= -9        then say 'failed in test          7 '
if  j \= 5         then say 'failed in test          7 '

/* test 8 */
 j=0;i=0
 do i=1 to 8 until i>3; j=j+1; end
if  i \= 4         then say 'failed in test          8 '
if  j \= 4         then say 'failed in test          8 '

/* test 9 */
j=0
do j+7 until j=12
j=j+1; end
if  j \= 7         then say 'failed in test          9 '

/* test 10 */
j=0
do j=1 to j+97 until j=12
  j=j+1; end
  if j\=12         then say 'failed in test          10 '

  /* test 11 */
  i=0
  do i=1 to 45; end
  if i\=46         then say 'failed in test          11 '

  /* test 12 */
i=0
loop 10
i=i+1
end
if  i \= 10        then say 'failed in test          12 '

 /* test 13 */
j=0
loop forever
  i=i+1; j=j+1
  if j=100 then leave
end
if  j \= 100 then say 'failed in test          13 '

 /* test 14 */
j=0;i=0
loop until i=15
i=i+1; j=j+1
end
if  i \= 15        then say 'failed in test          14 '
if  j \= 15        then say 'failed in test          14 '

/* test 15 */
j=0;i=0
loop while i<12
i=i+1; j=j+1
end
if  i \= 12        then say 'failed in test          15 '
if  j \= 12        then say 'failed in test          15 '


/* test 16 */
j=0;i=0
loop while i<12
  i=i+1
  if i = 11 then leave
  j=j+1
end
if  i \= 11        then say 'failed in test          16 '
if  j \= 10        then say 'failed in test          16 '

/* test 17 */
  j=0; flag='1'; fail=''
  loop i  =  1 to 12 by 2 while(i<=7)
    j=j+1; end
if  i \= 9         then say 'failed in test          17 '
if  j \= 4         then say 'failed in test          17 '

/* test 18 */
  j=0;i=0
  loop i=0-1 to(10-22)by 20-22 until i<-7
    j=j+1; end
if  i \= -9        then say 'failed in test          18 '
if  j \= 5         then say 'failed in test          18 '

/* test 19 */
 j=0;i=0
 loop i=1 to 8 until i>3; j=j+1; end
if  i \= 4         then say 'failed in test          19 '
if  j \= 4         then say 'failed in test          19 '

/* test 20 */
j=0
loop j+7 until j=12
j=j+1; end
if  j \= 7         then say 'failed in test          20 '

/* test 21 */
j=0
loop j=1 to j+97 until j=12
  j=j+1; end
  if j\=12         then say 'failed in test          21 '

  /* test 22 */
  i=0
  loop i=1 to 45; end
  if i\=46         then say 'failed in test          22 '


say 'Loop OK'
