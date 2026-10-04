options levelc
say 'main=' || arg()
call probe 'a',,'',;
say 'function=' || retrieve('xy')
exit

probe:
procedure
say 'count=' || arg()
say 'values=' || arg(1) || '|' || arg(2) || '|' || arg(3) || '|' || arg(4)
say 'exists=' || arg(1,'e') || arg(2,'e') || arg(3,'e') || arg(4,'e')
say 'omitted=' || arg(1,'o') || arg(2,'o') || arg(3,'o') || arg(4,'o')
arg first
say 'again=' || arg(1) || '|' || first
return

retrieve:
procedure
return arg() || ':' || arg(1)
