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
