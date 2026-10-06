options levelc
call levelc_call_alias.typedentry
say 'alias=' || result
if 0 then call levelc_call_alias_provider.typedentry
say 'done'
