/* C2X */
options levelb
import rxfnsb

errors=0
/* These from the Rexx book. */
/* EBCDIC
if c2x('72s') \= 'F7F2A2' then say 'failed in test */
if c2x('0123'x) \= '0123' then do
  errors=errors+1
  say 'failed in test 1: c2x('0123'x) =' c2x('0123'x)
end
/* These from Mark Hessling. */
/* if c2x( 'foobar') \= '666F6F626172' then say 'failed in test 3 ' EBCDIC */
if c2x( '' )\= '' then do
  errors=errors+1
  say 'failed in test 2: c2x( '' )= ' c2x( '' )
end

if c2x( '101'x )\= '0101' then do
  errors=errors+1
  say 'failed in test 3: c2x( '101'x )= ' c2x( '101'x )
end

if c2x( '48656c6c6f'x )\= '48656C6C6F' then do
  errors=errors+1
  say 'failed in test 4: c2x( '48656c6c6f'x ) = ' c2x( '48656c6c6f'x )
end

if c2x( 'ffff'x )\= 'FFFF' then do
  errors=errors+1
  say 'failed in test 5: c2x( 'ffff'x ) = ' c2x( 'ffff'x )
end

if c2x( 'ffffffff'x )\= 'FFFFFFFF' then do
  errors=errors+1
  say 'failed in test 6: c2x( 'ffffffff'x ) = ' c2x( 'ffffffff'x )
end

return errors<>0
