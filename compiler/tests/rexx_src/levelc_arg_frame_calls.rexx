options levelc
call probe 'a',,'c'
call probe '', 'b'
call probe ,'b',;
say pair(,'b')
say pair('a',)
exit

probe:
procedure
arg first, second, third
say first '/' second '/' third
arg again
say again
return

pair:
procedure
arg x,y
return x || ':' || y
