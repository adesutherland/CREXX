options levelc
arg main_first main_rest
say 'main' main_first main_rest
call probe 'a b',,'q:r'
call gaps 'one','two','three'
exit

probe:
procedure
arg first second, skipped, third ':' fourth
say 'probe' first second '/' skipped '/' third fourth
arg again . remaining, , entire
say 'repeat' again '/' remaining '/' entire
say 'presence' arg() arg(1,'E') arg(2,'O') arg(3,'E')
return

gaps:
procedure
arg x,,z
say 'gaps' x z
return
