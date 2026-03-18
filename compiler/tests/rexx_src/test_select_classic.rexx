/* Classic SELECT test */
options levelb

a = 10
select
  when a = 5 then say "Five"
  when a = 10 then say "Ten"
  otherwise say "Other"
end
