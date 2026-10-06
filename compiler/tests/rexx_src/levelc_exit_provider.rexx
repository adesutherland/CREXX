options levelc
arg mode
if mode = 'VALUE' then call inner
if mode = 'ONCE' then exit track()
if mode = 'EMPTY' then exit ''
exit
inner:
procedure
exit '雪' || x2c('00')
track:
say 'evaluated-once'
return 'once'
