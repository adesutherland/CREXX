/* C-style SELECT test */
options levelb

a = 10
select a
  when 5 then say "Five"
  when 10 then do
    say "Ten"
  end
  otherwise say "Other"
end
