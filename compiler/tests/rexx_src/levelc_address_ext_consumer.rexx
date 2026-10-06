options levelc
address system with output stem saved.
address bogus 'oops'
call levelc_address_ext_provider
say 'caller=' || address() || '/' || address('O') || '/' || rc || '/' || .rc || '/' || .rs
