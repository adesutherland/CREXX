options levelc
say 'count=' || arg()
say 'exists=' || arg(1,'E') || '|' || arg(2,'E')
say 'raw=' || arg(1) || '|' || arg(2)
arg first, rest
say 'parsed=' || first || '|' || rest
exit
