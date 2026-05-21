options levelb

errors = 0
choice = 2
classic_seen = 0
cstyle_seen = 0
nested_seen = 0

select
  when choice = 1 then errors = errors + 1
  when choice = 2 then classic_seen = 1
  otherwise errors = errors + 1
end

select choice
  when 1 then errors = errors + 1
  when 2 then cstyle_seen = 1
  otherwise errors = errors + 1
end

select
  when classic_seen = 1 & cstyle_seen = 1 then do
    select choice
      when 2 then nested_seen = 1
      otherwise errors = errors + 1
    end
  end
  otherwise errors = errors + 1
end

if classic_seen \= 1 then do
  errors = errors + 1
  say "Classic SELECT branch did not run"
end

if cstyle_seen \= 1 then do
  errors = errors + 1
  say "C-style SELECT branch did not run"
end

if nested_seen \= 1 then do
  errors = errors + 1
  say "Nested SELECT branch did not run"
end

if errors = 0 then say "PASS: select"
return errors <> 0
