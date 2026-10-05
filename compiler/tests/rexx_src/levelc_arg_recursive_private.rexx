options levelc
call recurse 'a b',,'z',3
exit
recurse:
procedure
arg first, gap, last, depth
say 'enter=' || depth || ':' || first || '|' || gap || '|' || last || '|' || arg(2,'E')
if depth > 1 then call recurse first || 'x',,last,depth - 1
arg again
say 'leave=' || depth || ':' || again || '|' || arg(2,'E')
return
