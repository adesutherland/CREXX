/* rexx */
options levelb
import rxfnsb

x='the quick brown fox jumps over the lazy dog'
len=length(x)

say time('l')
k=0
do j=1 to 100000
   do  i=1 to len
       b=substr(x,i,1)
       k=k+1
   end
end
say k time('l')
return 0
