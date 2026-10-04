options levelc
n = 0
value = 'old'
select
  before_first:
  when 0 then say 'bad first'
  between_arms:
  when hit() then value = 'new'
  when hit() then say 'bad late condition'
  otherwise say 'bad otherwise'
end
say value n
select
  when 0 then say 'bad empty'
  otherwise
end
say 'empty'
select
  when 0 then say 'bad inline'
  otherwise select
    when 1 then say 'inline'
  end
end
select
  when 0 then say 'bad list'
  otherwise
    parse value 'red blue' with first second
    say first second
    call bump
end
say n
if 1 then select
  when 1 then do
    select
      when 0 then say 'bad nested'
      otherwise say 'nested'
    end
  end
  otherwise say 'bad outer'
end
do i = 1 to 3
  select
    when i = 1 then iterate
    when i = 3 then leave
    otherwise say i
  end
end
select
  when 1 then ; anchor: say 'label'
end
select
  when 1 then say local()
end
select
  when 1 then options ''
end
select
  when 1 then drop value
end
say value
exit

hit: procedure expose n
  n = n + 1
  return 1

bump: procedure expose n
  n = n + 1
  return

local: procedure
  select
    when 0 then return 'bad local'
    when 1 then return 'local'
    otherwise return 'bad otherwise'
  end
  return 'unreachable'
