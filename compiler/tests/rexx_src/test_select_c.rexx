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


/* Missing OTHERWISE (NOP) */
select a
  when 999 then say "Should not print"
end

/* Nested SELECTs */
b = 5
select a
  when 10 then do
    select b
      when 5 then say "Nested 5"
      otherwise say "Nested Other"
    end
  end
  otherwise say "Outer Other"
end

/* Variable scope inside SELECT */
select a
  when 10 then do
    scope_var = .int
    scope_var = 42
    say "Scope: " scope_var
  end
  otherwise say "Other"
end
