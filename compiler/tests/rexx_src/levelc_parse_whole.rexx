options levelc
parse arg main_argument
say 'main-arg=' || length(main_argument)
queue 'MiXeD queued'
parse pull q1 qrest
say 'pull=' || q1 || '|' || qrest
queue 'split source'
parse upper pull up,,missing
say 'upperpull=' || up || '|' || length(missing)
queue 'still queued'
parse linein inputline
say 'linein=' || inputline
parse pull queuedline
say 'queue-after-linein=' || queuedline
parse linein
parse linein eofline
say 'eof=' || length(eofline)
queue 'discarded by bare parse'
parse pull
parse pull eofpull
say 'pull-eof=' || length(eofpull)
parse version platform bits build date
say 'version=' || (length(platform) > 0) || '|' || (bits = '64' | bits = '32') || '|' || (length(build) > 0)
parse source system how filename
say 'source=' || how || '|' || (pos('parse_whole.rexx', filename) > 0)
stem. = 'default'
stem.item = 'CaSe value'
parse var stem.item first rest
say 'compound=' || first || '|' || rest
parse upper var stem. stemvalue
say 'stem=' || stemvalue
source = 'one two three'
parse var source first source
say 'overwrite=' || first || '|' || source
parse value 'ab cd' with value_first,,value_third
say 'value=' || value_first || '|' || length(value_third)
parse value with empty_value
say 'empty=' || length(empty_value)
parse upper value 'é🙂 mixed' with unicode_word unicode_rest
say 'unicode=' || unicode_word || '|' || unicode_rest
source = '410042'x
parse var source nul_value
say 'binary=' || c2x(nul_value)
call routine 'MiXeD',, 'Third'
call binaryroutine '410042'x
call exposedroutine 'Visible value'
say 'exposed=' || observed
call recursive 'CaSe', 2
signal on novalue name missing_source
parse var absent_value ignored
say 'unexpected-novalue'
exit
routine: procedure
parse arg first,,third
say 'args=' || first || '|' || third
parse upper arg uppercase
say 'upperarg=' || uppercase
parse source system how filename
say 'local-source=' || how
return
binaryroutine: procedure
parse arg binary_value
say 'binary-arg=' || c2x(binary_value)
return
exposedroutine: procedure expose observed
parse arg observed
return
recursive: procedure
parse arg case_value, depth
say 'recursive=' || case_value || '|' || depth
if depth > 0 then call recursive case_value, depth - 1
return
missing_source:
say 'novalue=' || condition('C')
exit
