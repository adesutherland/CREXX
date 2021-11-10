/* rexx */
options levelb

x='the quick brown fox jumps over the lazy dog'
len=43

say time('l')
k=0
do j=1 to 100000
   do  i=1 to len
       b=fakesubstr(x,i,1)
       k=k+1
   end
end
say k time('l')
return

time: procedure = .string
   arg option = ""

fakesubstr: procedure = .string
   arg string1 = .string, start = .int, length1 = 1, pad = ' '
return "x"
