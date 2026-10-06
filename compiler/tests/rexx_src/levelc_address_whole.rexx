options levelc
say 'init=' || address() || '/' || .rc || '/' || .rs
name = 'SYS' || 'TEM'
address value 'SYS' || 'TEM'
say 'value=' || address()
address value 'CRE' || 'XX' with output stem via.
say 'valuewith=' || address() || '/' || address('O')
address system ''
say 'empty=' || rc || '/' || .rc || '/' || .rs || '/' || address()
address system '   '
say 'blank=' || rc || '/' || .rc || '/' || .rs || '/' || address()
address bogus ''
say 'unknownempty=' || rc || '/' || .rc || '/' || .rs || '/' || address()
address system
address crexx with output stem saved.
say 'persistent=' || address() || '/' || address('O')
address
say 'swap=' || address() || '/' || address('O') || '!'
address
say 'swapback=' || address() || '/' || address('O')
if address() = 'CREXX' then address crexx 'echo If' with output stem ifout.
say 'if=' || ifout.0 || '/' || ifout.1
do index = 1 to 1
  address (name)
end
say 'dynamic=' || address()
select
  when address() = 'SYSTEM' then address crexx with output stem saved.
  otherwise say 'unexpected select'
end
say 'selected=' || address() || '/' || address('O')
address crexx 'echo Alpha' with output stem out.
say 'first=' || out.0 || '/' || out.1 || '/' || address('O')
address crexx 'echo Beta' with output append stem out.
say 'append=' || out.0 || '/' || out.1 || '/' || out.2
streamfile = 'levelc-address-one.txt'
address crexx with output stream streamfile
streamfile = 'levelc-address-two.txt'
say 'snapshot=' || address('O')
address crexx with output stem saved.
command_calls = 0
address crexx command_echo() with output stem eval.
say 'eval=' || command_calls || '/' || eval.0 || '/' || eval.1
address bogus 'echo hi'
say 'unknown=' || rc || '/' || .rc || '/' || .rs
call private
say 'after=' || rc || '/' || .rc || '/' || .rs || '/' || address()
call recurse 2
say 'postrec=' || address() || '/' || address('O')
address crexx with input normal output normal error normal
say 'normal=' || (address('I') = 'INPUT NORMAL ') ||,
                  (address('O') = 'REPLACE NORMAL ') ||,
                  (address('E') = 'REPLACE NORMAL ')
exit
private: procedure
  say 'private=' || rc || '/' || .rc || '/' || .rs || '/' || address()
  address crexx 'echo Child' with output stem child.
  say 'child=' || child.0 || '/' || child.1 || '/' || .rc || '/' || .rs
  return
recurse: procedure
  arg depth
  say 'recur=' || depth || '/' || address() || '/' || address('O')
  address system with output stem rec.
  if depth > 1 then call recurse depth - 1
  return
command_echo:
  command_calls = command_calls + 1
  return 'echo Once'
