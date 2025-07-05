/* GETPI Plugin Test */
options levelb
import stack
import rxfnsb
ll1.1='first'
say "Create " createll(ll1,"123")
call listitems ll1
do i=1 to 10
   say push(ll1,"Record "i)
end
call listitems ll1
say 'insert '
say insertitem(ll1,1,"Record 1a")
say insertitem(ll1,0,"Record 0a")
say insertitem(ll1,10,"Record 8a")
say 'after insert 'll1.0
call listitems ll1
say 'remove'
say delitem(ll1,1)
call listitems ll1
say 'move'
say 'move 7 99 'moveitem(ll1,7,99)
call listitems ll1
say 'move 9 0 'moveitem(ll1,8,0)
call listitems ll1
say 'find item'
r1=finditem(ll1,"10",7)
say r1
r1=finditem(ll1,"10",r1+1)
say r1
r1=finditem(ll1,"10",r1+1)
say r1
call listitems ll1

say "SWAP " swapitem(ll1,3,9)
call listitems ll1
do i=1 to ll1.0
   say "XPULL "i" "pull(ll1)
end
call listitems ll1

do i=1 to 15
   say 'Queue 'i' 'queue(ll1,"Line "i)
end
call listitems ll1
do i=1 to ll1.0
   say "XPULLQ "i" "pullq(ll1)
end
call listitems ll1