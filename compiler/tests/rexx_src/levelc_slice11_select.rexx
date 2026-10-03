options levelc
n = 0
select
  when 1 then say 'first'
  when mark() then say 'bad'
  otherwise say 'bad'
end
say n
select
  when 0 then say 'bad'
  when mark() then do
    say 'second'
    select
      when 0 then say 'bad'
      otherwise
        say 'nested'
        nop
    end
  end
  otherwise say 'bad'
end
say n
if 1 then select
  when 0 then say 'bad'
  otherwise say 'arm'
end
say pick()
exit
mark:
procedure expose n
n = n + 1
return 1
pick:
procedure
select
  when 0 then return 'bad'
  when 1 then return 'procedure'
  otherwise return 'bad'
end
return 'bad'
