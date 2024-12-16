/* REXX LList plugin */
options levelb
import llist
import rxfnsb

do i=1 to 15
   call addnode 0,"Record "i
end
say "Set LList to first" setNode(0,"first")

say "Set LList to 3 "setNode(0,"3")
say "current Node      "currentNode(0)
say "current Node ADDR "currentNodeAddr(0)
do forever
   entry=nextnode(0)
   if pos("$END-OF",entry)>0 then leave
   say "Next "entry
end
say "set LList to 4 "setNode(0,"4")
say "current Node      "currentNode(0)
say "current Node ADDR "currentNodeAddr(0)

do forever
   entry=prevnode(0)
   if pos("$TOP-OF",entry)>0 then leave
   say "Prev "entry
end

say "Set LList to -12 "setNode(0,"-12")
do forever
   entry=nextnode(0)
   if pos("$END-OF",entry)>0 then leave
   say "Next "entry
end
say "Set LList to +7 "setNode(0,"first")
say "Set LList to +7 "setNode(0,"+7")
do forever
   entry=nextnode(0)
   if pos("$END-OF",entry)>0 then leave
   say "Next "entry
end

do forever
   entry=nextnode(0)
   if pos("$END-OF",entry)>0 then leave
   say "Next "entry
end
say "Reset LList "setNode(0,"fIrst")
do forever
   entry=nextnode(0)
   if pos("$END-OF",entry)>0 then leave
   say "Next "entry
end
say "set LList "setNode(0,"LAst")
do forever
   entry=prevnode(0)
   if pos("$TOP-OF",entry)>0 then leave
   say "Prev "entry
end


call listnode(0)
call listllist(0)
call freellist(0)
