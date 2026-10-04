options levelc
say receive('a' || '00'x || 'b')
exit

receive:
procedure
arg value
say arg(1)
return value
