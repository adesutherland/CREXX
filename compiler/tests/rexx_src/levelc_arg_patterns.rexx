options levelc
separator=':'
call probe 'ab:cd', 'left right'
exit

probe:
procedure expose separator
arg first (separator) second, , third
say 'pattern' first second third
arg , pair_first pair_rest
say 'leading' pair_first pair_rest
arg first,;
say 'trailing' first
arg
say 'bare'
return
