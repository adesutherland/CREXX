/* REXX LList plugin */
options levelb
import llist
import rxfnsb

do i=1 to 10
   call push  0,"Record Push  "i
   call queue 0,"Record Queue "i
end
call Listllist 0

do i=1 to 5
   say i pull(0)
end
do i=1 to 5
   say i qpull(0)
end

exit

push: procedure
  arg llist = .int, record = .string
  call appnode llist,record
return
queue: procedure
  arg llist = .int, record = .string
  call prepnode llist,record
return

pull: procedure = .string
  arg llist = .int
  call setNode llist,"LAST"
  record=currentNode(llist)
  call removeNode(llist)
return record

qpull: procedure = .string
  arg llist = .int
  call setNode llist,"FIRST"
  record=currentNode(llist)
  call removeNode(llist)
return record


/*
do i=1 to 5
   call addnode 0,"Record "i
end
call listLlist 0

say "Set LList to 3. record" d2x(setNode(0,"3"))
say "**** remove 3. item "removeNode(0)
call listLlist 0
say "Set LList to first" d2x(setNode(0,"first"))
do i=1 to 3
   say "+++ "currentNode(0)
   say "**** remove item "i removeNode(0)
   call listLlist 0
end



say "Set LList to first" setNode(0,"first")
say "**** insert before first -1 "insertNode(0,"first -1 Record","before")
call listLlist 0
say "**** insert before first -2 "insertNode(0,"first -2 Record","before")
call listLlist 0
say "**** insert before first -3 "insertNode(0,"first -3 Record","before")
call listLlist 0
say "**** insert after last +1   "insertNode(0,"first +1 Record","after")
call listLlist 0
say "**** insert after last +2   "insertNode(0,"first +2 Record","after")
call listLlist 0
say "**** insert after last +3   "insertNode(0,"first +3 Record","after")
call listLlist 0

say "Set LList to first" setNode(0,"Last")

say "**** insert after last "insertNode(0,"Last+1 Record","after")
call listLlist 0
say "Set LList to first" setNode(0,"first")
say "*** insert after 1 "insertNode(0,"first.1 Record","after")
call listLlist 0
say "Set LList to first" setNode(0,"Last")
say "**** insert before last "insertNode(0,"Last -1 Record","before")
call listLlist 0

exit
say "Set LList to 3 "setNode(0,"3")
say "current Node      "currentNode(0)
say "current Node ADDR "currentNodeAddr(0)
do forever
   entry=nextnode(0)
   if pos("$END-OF",entry)>0 then leave
   say "Next "entry
end
call listLlist 0
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
*/