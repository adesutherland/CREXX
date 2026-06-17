options levelb

main: procedure
  parse value "alpha,beta" with left "," right
  say left
  say right

  if 1 then parse value "one two" with first second
  say first
  say second

  if 0 then say "skip"
  else parse value "red blue" with first second
  say first
  say second

  select
    when 0 then say "skip"
    otherwise parse value "up down" with first second
  end
  say first
  say second

  select
    when 1 then parse value "hot cold" with first second
    otherwise say "skip"
  end
  say first
  say second
  return
