/* rexx */
options levelb

namespace _rxsysb

/* Translate date dd mm yy into Julian Day Number */
_jdn: Procedure = .int
  arg day = .int, month = .int, year = .int
  a=(14-month)%12
  m=month+12*a-3
  y=year+4800-a
  jdn=day+(153*m+2)%5+365*y
  jdn=jdn+y%4-y%100+y%400-32045
return jdn