
/* These are the testcases for the Classic Rexx 4.02 compatible bRexx/NetRexx Date() */
/* trace 'r' */
call load "date_fn.rexx"
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
say "date('B','10 Mar 1962') --> 716308 :" date('B','10 Mar 1962')
say "date('W','10 Mar 1962','N') --> Saturday :" date('W','10 Mar 1962','N')
say "date('W','716308','B') --> Saturday :" date('W','716308','B')
say "date('S','716308','B')  --> 10 Mar 1962 :" date('S','716308','B')
say "date('c','1 Feb 2021')       ==> 7703        :" date('c','1 Feb 2021')
say "date('J','18 Jan 2021')      ==> 2021018     :" date('j','18 Jan 2021')
say "date('J','10 Mar 1962')      ==> 1962069     :" date('j','10 Mar 1962')

/* say '' */
/* say 'with separators specified' */
/* say "date('s','716308','b','/')   ==> 1962/03/10  :" date('s','716308','b','/') */
/* say "date('s','716308','b','-')   ==> 1962-03-10  :" date('s','716308','b','-') */
/* say "date('w',7688,'c')           ==> Sunday      :" date('w',7688,'c') */
/* say "date('c','1 Feb 2021')       ==> 7703        :" date('c','1 Feb 2021') */
/* say "date('J','18 Jan 2021')      ==> 2021018     :" date('j','18 Jan 2021') */
/* say "date('J','10 Mar 1962')      ==> 1962069     :" date('j','10 Mar 1962') */

