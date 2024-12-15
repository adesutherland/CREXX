/* rexx test optimise do loop sample */
options levelb
start=0
xend=0
f1=1.0
f2=3.14
f3=6.28
f4=f2
assembler mtime start
do i=1 to 100000000
   if f1>f2+i then say "OK"
end
say f1
assembler mtime xend
etime=xend-start
say etime/1000000

