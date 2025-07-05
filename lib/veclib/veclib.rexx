/* veclib for CREXX
  Mike Beer
*/

options levelb
namespace veclib expose __size __min __max __quad_io __iota __pick,
   __sum __each __reduce __vec2s __s2vec  __alfasort __numsort  __indexvec
import rxfnsb

__getenv: procedure=.string
arg search_var=.string

myvar = .string
assembler getenv myvar,search_var
return myvar

__quad_io: procedure=.int
  arg io=2
    if io=2 then do
       x=__getenv("index_origin")
       if x=1 | x=0 then return x
       else return 1
    end
   if io=1 | io=0 then do
      say "set []IO to" io
   end
return io

__min: procedure=.int
   arg line=.string
   /* _min
      input:
      a - vector
      output:
      min
   */

   rho = __size(line)

   min=word(line,1)

   do i=2 to rho
      x = word(line,i)
      min = min(min,x)
   end

return min

__size: procedure=.int
   arg line=.string
return words(line)

__max: procedure=.int
   arg line=.string
   /* _mmax
      input:
      a - vector
      output:
      max
   */

   rho = __size(line)

   max=word(line,1)

   do i=2 to rho
      x = word(line,i)
      max = max(max,x)
   end

return max

/* iota - create index vector */
__iota: procedure=.string
   arg n=.int,io=2

say "iota"
quad_io = __getenv("index_origin")
if quad_io=""  then quad_io=1
say quad_io
if io=1 | io=0 then quad_io = io
say quad_io

ret=""

do i=1 to n
   ret=ret i - (1-quad_io)
end

ret=strip(ret)

return ret



/* pick elements out of a vector
   e.g. a=1 5 7 9
   say pick(a,1 3) ==> 1 7
*/
__pick: procedure = .string
   arg vec=.string, items=.string

ret=""

do i=1 to words(items)
   x=word(items,i)
   ret=ret word(vec,x)
end

ret=strip(ret)

return ret



/* sum = +/... */
__sum: procedure=.int
   arg vec=.string

ret=0

do i=1 to words(vec)
   ret= ret + word(vec,i)
end

return ret

__reduce: procedure=.int
   arg op=.string, vec=.string

ret=word(vec,1)
   do i=2 to words(vec)
      w=word(vec,i)
      if op="+" then ret= ret + w
      if op="-" then ret= ret - w
      if op="*" then ret= ret * w
      if op="/" then ret= ret / w
      if op="%" then ret= ret % w
      if op="//" then ret= ret // w
   end
return ret

__each: procedure = .string
   arg a=.string,op=.string,b=.string
return a
/*
/* each.rex
   vector 1
   operation (e.g. ":")
   vector 2
*/

ret=""

select
   when b="" then do /* monadic use */
      max = words(a)
      do i=1 to max
         w=word(a,i)
         s=changestr("%",op,w)
         x ="r="s
         interpret x
         ret = ret r
         end
      end
   when words(a)=1 then do  /* skalar a with vector b */
      do i=1 to words(b)
         ret = ret a||op||word(b,i)
      end
   end


   otherwise do /* dyadic use */
      do i=1 to words(a)
         ret = ret word(a,i)||op||word(b,i)
      end
   end /*otherwise */
end /* select */


return strip(ret)
*/


__s2vec: procedure=.string[]
arg v=.string
   do i=1 to words(v)
      s[i] = word(v,i)
   end
return s

__vec2s: procedure=.string
arg s=.string[]

   ret=""
   do i=1 to s[0]
      ret=ret s[i]
   end

   ret=strip(ret)
return ret


/********************************* SORT *******************************/

__alfasort: procedure=.string[]
arg x=.string[]
max = x[0]

h = 1
Do Until h > max
  h = 3 * h + 1
End

Do Until h = 1
   h = h % 3
   Do i=h+1 To max
      V = x[i]
      j = i
      jmh = j - h
      Do While x[jmh] > V
         x[j] = x[jmh]
         j = j - h
         If j <= h Then leave
         jmh = j - h
      End
      x[j] = V
   End
End

Return x


__numsort: procedure=.string[]
arg x=.string[]
max = __max(__vec2s(x))
digits = length(max||"")

max = x[0]
do i=1 to max
   x[i]=right(copies("0",digits)||x[i],digits)
end


h = 1
Do Until h > max
  h = 3 * h + 1
End

Do Until h = 1
   h = h % 3
   Do i=h+1 To max
      V = x[i]
      j = i
      jmh = j - h
      Do While x[jmh] > V
         x[j] = x[jmh]
         j = j - h
         If j <= h Then leave
         jmh = j - h
      End
      x[j] = V
   End
End

do i=1 to max
   x[i]=x[i]+0
end

Return x
/* indexvec - indexvektor bilden
   input:
   - string
   - reverseflag
*/

__vecpos: procedure=.int
arg needle=.string,vec=.string[]
ret=0
do i=1 to vec[0]
   if vec[i]=needle then return i
end
return ret


__indexvec: procedure=.string
arg s=.string[],reverse=""

max = s[0]

type="NUM"
do i=1 to max
   w = s[i]
   if pos(left(w,1),"0123456789-")=0  then do
      type="CHAR"
      leave
      end
end

if type="NUM" then vec = __numsort(s)
   else vec = __alfasort(s)

ret=""

if reverse="" then
   do i = 1 to max
      x = vec[i]
      p = __vecpos(x,s)
      if p>0 then vec[p] = "*"
      ret = ret p
   end
else
   do i = max to 1 by -1
      x = vec[i]
      p = __vecpos(x,s)
      if p>0 then vec[p] = "*"
      ret = ret p
   end

ret=strip(ret)
return ret
