/* C-style SELECT test */
options levelb

a = 10
select a
  when 5 then say "Five"
  when 10 then do
    say "Ten"
  end
  otherwise
    say "Other"
end

select a
  when 1 then say "One"
  otherwise say "Other Same Line"
end

select a
  when 2 then say "Two"
  otherwise
    say "Other1"
    say "Other2"
end

select a
  when 3 then say "Three"
  otherwise do
    say "Other in Do"
  end
end
