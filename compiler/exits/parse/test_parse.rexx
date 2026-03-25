options levelb
import rxfnsb

main: procedure
  myArray=.string[]

  parse upper value "4711Alice Johnson   1264.19EUR London UK" 1 id 5 name 21 amount 28 currency city country
  say 111 "<"id">"
  say 222 "<"name">"
  say 333 "<"amount">"
  say 444 "<"currency">"
  say 555 "<"city">"
  say 666 "<"country">"

  parse lower value "9999John Smith      9999.99AUD Sydney" with id 5 name 21 amount 28 currency 31 city

  say 611 "<"id">"
  say 622 "<"name">"
  say 633 "<"amount">"
  say 644 "<"currency">"
  say 655 "<"city">"

  Line= "To be, or not to be?"
  parse var line with  w1 ',' w2
  say 111 w1
  say 222 w2
return