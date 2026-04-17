options levelb

main: procedure
  src = "alpha,beta"
  parse upper value src with left "," right trim into parsed
  parse var src with first "," second
  return
