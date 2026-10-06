options levelc
say 'provider-before=' || address() || '/' || address('O') || '/' || .rc || '/' || .rs
address crexx 'echo Child' with output stem child.
say 'provider-command=' || child.0 || '/' || child.1 || '/' || .rc || '/' || .rs
address crexx
say 'provider-after=' || address() || '/' || address('O') || '!'
return
