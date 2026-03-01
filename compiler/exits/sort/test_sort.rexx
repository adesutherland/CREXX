options levelb
import rxfnsb

main: procedure
a=.string[]
j=0
max=11
do i=1 to max
   j=j+1
   a[i]="Line "right(i,6,'0')
end

say 'Before'
do i=1 to a[0]
   say i a[i]
end
k=1
myOrder='DESC'
call test a,k+2,myorder

say 'After'
do i=1 to a[0]
   say i a[i]
end
return
test:procedure=.int
  arg expose a=.string[],offset=1,order='ASC'
  debug=1
  Sort a offset order 1
return 0
