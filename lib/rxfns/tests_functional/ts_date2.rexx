/* These are the testcases for the Classic Rexx 4.02 compatible bRexx/NetRexx Ibmdate() */
/* trace 'r' */
options levelb
import rxfnsb

errors=0
say 'Date single options'
say "date() -->" date()
say "date('W') -->" date('W')
say "date('S') -->" date('S')
say "date('E') -->" date('E')
say "date('J') -->" date('J')
say "date('D') -->" date('D')
say "date('C') -->" date('C')
say "date('B') -->" date('B')
say "date('M') -->" date('M')
say "date('N') -->" date('N')
say "date('O') -->" date('O')
say "date('WEDNEWS') -->" date('WEDNEWS')
say "date('wednews') -->" date('wednews')


/* say '' */
/* say 'Time (only has single option)' */
/* say "time()    -->" time() */
/* say "time('C') -->" time('C') */
/* say "time('E') -->" time('E') */
/* say "time('H') -->" time('H') */
/* say "time('L') -->" time('L') */
/* say "time('M') -->" time('M') */
/* say "time('N') -->" time('N') */
/* say "time('O') -->" time('O') */
/* say "time('R') -->" time('R') */
/* say "time('E') -->" time('E') */
/* say "time('S') -->" time('S') */

say''
say 'Date input/conversion options'
if date('B','10 Mar 1962') \= 716308 then do
  errors=errors+1
  say "DATE() failed in test 1: date('B','10 Mar 1962')" date('B','10 Mar 1962') "but must be 716308"
end
if date('W','10 Mar 1962','N') \= 'Saturday' then do
    errors=errors+1
  say "DATE() failed in test 2: date('W','10 Mar 1962','N')" date('B','10 Mar 1962') "but must be 'Saturday'"
end
if date('W','716308','B') \= 'Saturday' then do
    errors=errors+1
  say "Date() failed in test 3: date('W','716308','B')" date('W','716308','B') "but must be 'Saturday'"
end
if date('S','716308','B')  \= '19620310' then do
  errors=errors+1
  say "DATE() failed in test 4: date('S','716308','B')" date('S','716308','B') "but must be '19620310'"
end
/* if date('c','1 Feb 2021') \= 7703 then do */
/*   errors=errors+1 */
/*   say "DATE() failed in test 5: date('c','1 Feb 2021')" date('c','1 Feb 2021') "but must be 7703" */
/* end */
if date('J','18 Jan 2021') \= 2021018 then do
  errors=errors+1
  say "DATE() failed in test 6: date('j','18 Jan 2021')" date('J','18 Jan 2021') "but must be 2021018"
end
if date('J','10 Mar 1962')  \= '1962069' then do
  errors=errors+1
  say "DATE() failed in test 7: date('j','10 Mar 1962')" date('J','10 Mar 1962') "but must be 1962069"
end

/* say '' */
/* say 'with separators specified' */
if date('s','716308','b','/')  \= '1962/03/10' then do
  errors=errors+1
  say "DATE() failed in test 8: date('s','716308','b','/')" date('s','716308','b','/') "but must be '1962/03/10'"
end
if date('s','716308','b','-') \= '1962-03-10' then do
  errors=errors+1
  say "DATE() failed in test 9: date('s','716308','b','-')" date('s','716308','b','-') "but must be '1962-03-10'"
end
/* if date('w','7688','c') \= 'Sunday' then do */
/*   errors=errors+1 */
/*   say "DATE() failed in test 10: date('w',7688,'c')" date('w','7688','c') "but must be 'Sunday'" */
/* end */
/* if date('c','1 Feb 2021') \= 7703 then do */
/*   errors=errors+1 */
/*   say "DATE() failed in test 11: date('c','1 Feb 2021')" date('c','1 Feb 2021') "but must be 7703" */
/* end */
if date('J','18 Jan 2021') \= '2021018' then do
  errors=errors+1
  say "DATE() failed in test 12: date('j','18 Jan 2021')" date('j','18 Jan 2021') "but must be '2021018'"
end
if date('J','10 Mar 1962') \= 1962069 then do
  errors=errors+1
  say "DATE() failed in test 13: date('j','10 Mar 1962')" date('j','10 Mar 1962') "but must be 1962069"
end
if date('N','2022166','J') \= '15 Jun 2022' then do
  errors=errors+1
  say "DATE() failed in test 14: date('N','2022166','J')" date('N','2022166','J') "but must be 15 Jun 2022"
end

say 'end of date2 test'
say errors 'errors'
return errors<>0

