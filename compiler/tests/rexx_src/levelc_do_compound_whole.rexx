options levelc
i='a'
old='a'
new='b'
do a.i=1 to 2
  say 'body=' || a.old
  i='b'
  a.new=10
end a.i
say 'after=' || a.old || '|' || a.new

row='r'
col='c'
saved='c'
alternate='d'
do grid.row.col=1 to 2
  say 'multi=' || grid.row.saved
  col='d'
  grid.row.alternate=7
end grid.row.col
say 'multi-after=' || grid.row.saved || '|' || grid.row.alternate

i='a'
old='a'
new='b'
a.old=90
a.new=80
say 'header-before' a.old a.new i
do a.i=1 to move()
  say 'header-body' a.old a.new i
end a.i
say 'header-after' a.old a.new i
exit
move: procedure expose i
  i='b'
  return 2
