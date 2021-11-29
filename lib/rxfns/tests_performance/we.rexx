/* rexx */
options levelb

/* rexx */
today = Date('b')
xv = today - date('b','23 Sep 1958')
xr = today - date('b','10 Mar 1962')
xa = today - date('b','23 Aug 1967')
xm = today - date('b','19 Sep 1992')
xs = today - date('b','26 Mar 1990')
xf = today - date('b','15 Apr 1934')
xd = today - date('b','6 Oct 1939')
md = today - date('b','26 May 2017')
say 'Today''s date is:' date() /* ', day ' date('j') */
say 'Venetia was born on 23 Sep 1958:' xv 'days ago.'
say 'René    was born on 10 Mar 1962:' xr 'days ago.'
/* say */
say 'We were married on 19 Sep 1992, ' xm 'days ago.'
say 'Percentage of her life Venetia is married to René:' xm/xv*100
say 'Percentage of his life René is married to Venetia:' xm/xr*100
say 'Days       of his life René is together w Venetia:' xs 'days'
say 'Percentage of his life René is together w Venetia:' xs/xr*100
say 'Percentage of her life Venetia is together w René:' xs/xv*100
/* say */

do i=today  to 900000
  xv = I - date('b','23 Sep 1958')
  xm = I - date('b','19 Sep 1992')
  p = xm/xv*100
  if p = 50 then
  do
    say date('n',i,'b') 'is the date Venetia is married for half of her life to René.'
    leave
  end
end
do i=today  to 900000
  xv = I - date('b','23 Sep 1958')
  xy = I - date('b','26 Mar 1990')
  p = xy/xv*100
  if p = 50 then
  do
    say date('n',i,'b') 'is the date Venetia is together for half of her life with René.'
    leave
  end
end
/* do i=today  to 900000 */
/*   xr = I - date('b','10 Mar 1962') */
/*   xm = I - date('b','19 Sep 1992') */
/*   p = xm/xr*100 */
/*   if p = 50 then */
/*   do */
/*     say date('n',i,'b') 'is the date René is married for half of his life to Venetia.' */
/*     leave */
/*   end */
/* end */
/* do i=today-9000  to 900000 */
/*   xr = I - date('b','10 Mar 1962') */
/*   xz = I - date('b','26 Mar 1990') */
/*   p = xz/xr*100 */
/*   if p = 50 then */
/*   do */
/*     say date('n',i,'b') 'is the date René is together for half of his life with Venetia.' */
/*     leave */
/*   end */
/* end */
/* say */
say 'Frans   was born on 15 Apr 1934:' xf 'days ago.'
say 'Dolly   was born on 06 Oct 1939:' xd 'days ago.'
say 'Dolly   died     on 26 May 2017:' md 'days ago.'
say 'Dolly   has lived              :' xd-md 'days  '
say 'Aimée   was born on 23 Aug 1967:' xa 'days ago.'
twentythou = date('b','23 Aug 1967')+20000
say 'Aimée   will have her 20000 on :' date('n',twentythou,'b')
 
/* Prototype functions */
date: Procedure = .string
   arg iFormat = "", idate = "", oFormat = ""

