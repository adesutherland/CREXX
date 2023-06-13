/* rexx test optimise do loop sample */
options levelb
start=0
xend=0
assembler mtime start
do i=1 to 100000000
   x=i
end
assembler mtime xend
etime=xend-start
say etime/1000000

