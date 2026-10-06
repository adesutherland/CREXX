options levelc
parse source system how filename
say 'main=' || how || '|' || (pos('consumer source file.rexx', filename) > 0)
call levelc_parse_source_provider '410042'x,, 'é🙂'
say 'subresult=' || result
exit
