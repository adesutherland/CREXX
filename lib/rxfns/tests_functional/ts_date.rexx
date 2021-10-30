/* rexx test abs bif */
options levelb
say "test Date"

say date('USA','31/12/2021','XEUROPEAN')
say date('WEEKDAY','31/12/21','EUROPEAN')
say date('ORDERED','30.10.20','GERMAN')
say date('XGERMAN','30.10.20','GERMAN')
say date('SORTED','10/30/21','USA')
say date('JULIAN','30/10/21','EUROPEAN')
say date('JDN','10/30/21','EUROPEAN')
say date('','10/30/21','USA')

return

/*
say "Weekday "_dateo(iNorm,'WEEKDAY')
say "Days in Century "_dateo(iNorm,'CENTURY')
say "Long Date "_dateo(iNorm,'LONG')
say "Ordered Date "_dateo(iNorm,'ORDERED')
say "Default Format "_dateo(iNorm,"")
say "Month "_dateo(iNorm,"MONTH")
return /*_dateo(iNorm,oFormat) */
*/

/* Prototype functions */
date: Procedure = .string
   arg iFormat = .string, idate = .string, oFormat = .string

