/* rexx test abs bif */
options levelb
/* RXVM ts_date date _jdn _dateo _datei abbrev right word words wordindex wordpos pos substr length copies upper */
/* TODO */
say "test Date"

say "SEP "date('XGERMAN','31>12>2021','XEUROPEAN',"+$%",">")

say "date()" date()
say "date('WEDNESDAY')" date('wednesday')
say "date('USA')" date('USA')
say "date('normal')" date('n')
say "date('qualified')" date('QUALIFIED')
say "date('European')" date('EUROPEAN')
say "date('Epoch')" date('EPOCH')
say "date('xGerman')" date('XGERMAN')
say "date('Julian')" date('JULIAN')
say "date('Ordered')" date('ORDERED')
say "date('International')" date('INTERNATIONAL')
say "date('Sorted')" date('SORTED')

say date('USA','31/12/2021','XEUROPEAN')
d1=date('EPOCH','31/12/2021','XEUROPEAN')
say d1
say date('XEU',d1,'EPOCH')

say date('XDEC','31.12.21','xGerMAN')
say date('WEEKDAY','31/12/21','euroPEAN')
say date('QUALIFIED','31/12/21','EUROPEAN')
say date('EUROPEAN','Friday, DECEMBER 31, 2021','QUALIFIED',"$")

say date('ORDERED','30.10.20','GERMAN')
say date('XGERMAN','30.10.20','GERMAN')
say date('SORTED','09/20/1988','XUSA')

say date('JULIAN','20/09/88','EUROPEAN')
say date('JDN','10/30/21','EUROPEAN')
say date('','10/30/21','USA')
say date('USA','','XEUROPEAN')
say date('GERMAN','','XEUROPEAN')
say date('JULIAN','','XEUROPEAN')
say date('ORDERED','','XEUROPEAN')
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
   arg iFormat = "", idate = "", oFormat = "", osep="", isep=""

