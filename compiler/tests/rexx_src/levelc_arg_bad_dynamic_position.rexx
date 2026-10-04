options levelc
pos='Q'
say 'before'
call probe 'abc'
exit

probe:
procedure expose pos
arg first =(pos) second
say 'after'
return
