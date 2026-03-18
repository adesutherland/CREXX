/* Classic SELECT test */
options levelb

a = 10
select
  when a = 5 then say "Five"
  when a = 10 then say "Ten"
  otherwise
    say "Other"
end

select
  when a = 1 then say "One"
  otherwise say "Other Same Line"
end

select
  when a = 2 then say "Two"
  otherwise
    say "Other1"
    say "Other2"
end

select
  when a = 3 then say "Three"
  otherwise do
    say "Other in Do"
  end
end
