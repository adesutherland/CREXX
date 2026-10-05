options levelc
call routine
say 'main'
exit
routine:
say 'start'
do i = 1 to 2
  signal 'later'
  say 'skip'
end
later:
say 'later=' || sigl
return
