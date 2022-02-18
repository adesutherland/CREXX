/* rexx */
options levelb
/* the performance measure that started it all */
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
return

length: procedure = .int
  arg string1 = .string
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '
time: procedure = .string
arg option = ""