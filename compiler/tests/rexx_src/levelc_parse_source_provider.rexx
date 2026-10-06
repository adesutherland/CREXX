options levelc
parse source system how filename
say 'provider=' || how || '|' || (pos('levelc_parse_source_provider.rexx', filename) > 0)
parse arg first,,third
say 'provider-arg=' || c2x(first) || '|' || third
return 'ok'
