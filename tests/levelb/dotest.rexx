options levelb

/*- - D O - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
i=0
do 10
  i=i+1
  end
if i=10 then ok=ok ! 'Explicit Do'
        else say '*** Bad *** Explicit do'

j=0; m=0
do 5-2 while 1=1   /* possible DO block problem */
  m=m+2
  end
do while i<12
  i=i+1; j=j+1
  end
do k=1+2 to 3+1 while 1=1  /* checks storage allocation self-check */
  end
if i=12&j=2&k=5&m=6 then ok=ok ! 'Do While'
                    else say '*** Bad *** Do While'

j=0
do until i=15
  i=i+1; j=j+1
  end
if i=15&j=3 then ok=ok ! 'Do Until'
            else say '*** Bad *** Do Until'
say ok; ok='OK'

/* Test iterative Do loops of various kinds */
j=0; flag='1'
do i  =  1 to 12 by 2 while(i<=7)
  j=j+1; end
if j\=4 | i\=9 then flag='0'

j=0
do i=0-1 to(10-22)by 20-22 until i<-7
  j=j+1; end
if j\=5 | i\=-9 then flag='0'

j=0
do i=1 to 8 until i>3; j=j+1; end
if i><4 | j<>4 then flag='0'

j=0
do j+7 until j=12
  j=j+1; end
if j\=7 then flag='0'

j=0
do j+97 until j=12
  j=j+1; end
if j\=12 then flag='0'

do i=1 to 45; end
if i\=46 then flag='0'

do a.3 = 2 to 2  /* check loop var with substitution (was bug) */
  if a.3\=2 then flag='0'
  end
p=3
do a.p = 5 to 5
  if a.3\=5 then flag='0'
  end

j=0; do forever
   j=j+1; if j=6 then leave; end
if j\=6 then flag=0

/* Test iterative Do loops with non-integer arguments */
k=1
/* These should all iterate once */
do 0.9999999999    /* legal as will round to 1 */
  k=k+1
  end
flag=flag&k=2
do i=0.9999999999 to 1 by -1
  k=k+1
  end
flag=flag&k=3
do i=1 to 0.9999999999
  k=k+1
  end
flag=flag&k=4
/* Now a bit more complicated */
k=0
do i=-0.5 to 0.5 by 0.1
  k=k+1
  end
flag=flag&k=11
do i=0+0.5 to 0-0.5 by ' - 1E-1 '
  k=k+1
  end
flag=flag&k=22
k=0
do i=-0.5 to 0.5 by 0.1 until i>0.3
  k=k+1
  end
flag=flag&i=0.4 &k=10
k=0
do i=-5E-1 to 5E-1 by 0.1 while i<0.3
  k=k+1
  end
flag=flag&i=0.3 &k=8
k=0
do i=-5E-1 to 5E-1 by 0.1 for 1
  k=k+1
  end
flag=flag&k=1
k=0
do i=1 to 10 for 9
  k=k+1
  end
flag=flag&k=9 &i=10
k=0
do i=1 to 10 for 10
  k=k+1
  end
flag=flag&k=10&i=11
k=0
do i=1 to 10 for 11
  k=k+1
  end
flag=flag&k=10&i=11

trace ooo
if flag then ok=ok ! 'Do I=nn..'
        else say '*** Bad *** Do I=nn..'

i=1
j=3; do j=1 to j
   i=i+1
   end

if i=4 then ok=ok ! 'Do j=1 to j'
       else say '*** Bad *** Do J=1 to J'

j=0; k=0
do i=1 by 1 until j=10
  j=j+1
  if j=5 then leave
  if j=3 then do
    k=9
    iterate
    end
  k=k+1
  end
if j=5 & k=10 then flag=1
              else flag=0
k=0; l=0
do i=1 to 10
   k=k+1
   do j=6.0 to 1*1+(1-1)by -1
     l=l-1
     if j>3 then iterate j
     l=l-1
     if i=2 then leave i
            else leave j
     l=124 dead
     end
   k=k+5
   end
if l\=-10 | k\=7 then flag=0

if flag then ok=ok ! 'Leave/Iterate'
        else say '*** Bad *** Leave/Iterate'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
