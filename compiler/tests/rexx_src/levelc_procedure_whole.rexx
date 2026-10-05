options levelc
shared = 'parent'
call share
say 'share=' || shared
call private
say 'private=' || shared
key = 'b'
a.key = 'lower'
a.B = 'upper'
list = 'one  /  1bad  two'
one = '1'
two = '2'
ghost = 'G'
call mixed
say 'mixed-after=' || key || '|' || a.key || '|' || a.B || '|' || one || '|' || two || '|' || ghost
call reverse
say 'reverse-after=' || a.key || '|' || a.B
call stemmer
say 'stem-after=' || a.key || '|' || a.B
call compoundStem
say 'compound-stem-after=' || a.key || '|' || a.B
list = ''
call emptylist
say 'empty-indirect=' || list
exit
share:
  shared = 'shared'
  return
private:
  procedure
  shared = 'private'
  return
mixed:
  procedure expose key a.key one (list)
  say 'mixed-before=' || key || '|' || a.key || '|' || one || '|' || two || '|' || ghost
  drop a.key
  say 'mixed-dropped=' || a.key
  a.key = 'changed'
  key = 'B'
  say 'mixed-switch=' || a.key
  key = 'b'
  call nested
  say 'mixed-nested=' || a.key
  one = 'X'
  two = 'Y'
  ghost = 'private'
  return
nested:
  procedure expose key a.key
  a.key = 'deep'
  return
reverse:
  procedure expose a.key key
  a.key = 'local'
  return
stemmer:
  procedure expose a.
  a.B = 'stem-change'
  return
compoundStem:
  procedure expose key a.key
  a. = 'local-default'
  say 'compound-stem-set=' || a.key || '|' || a.B
  drop a.
  say 'compound-stem-drop=' || a.key || '|' || a.B
  a.key = 'restored'
  return
emptylist:
  procedure expose (list)
  list = 'changed'
  return
