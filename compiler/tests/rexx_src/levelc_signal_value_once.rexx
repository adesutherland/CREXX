options levelc
counter = 0
signal value choose()
say 'skip'
done:
say 'count=' || counter || ' sigl=' || sigl
return
choose:
counter = counter + 1
return 'done'
