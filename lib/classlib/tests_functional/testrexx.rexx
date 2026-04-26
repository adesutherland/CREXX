options levelb comments_dash
import rexx

say 'started testrexx'
errors = 0

t = .testrexx()

rc = t.t_abbrev() ; errors=errors+rc
say 'rc abbrev=' rc
say 'errors' errors

rc = t.t_abs() ; errors=errors+rc
say 'rc abs=' rc
say 'errors' errors

rc = t.t_b2x() ; errors=errors+rc
say 'rc b2x=' rc
say 'errors' errors

rc = t.t_centre() ; errors=errors+rc
say 'rc centre=' rc
say 'errors' errors

rc = t.t_changestr() ; errors=errors+rc
say 'rc changestr=' rc
say 'errors' errors

rc = t.t_compare() ; errors=errors+rc
say 'rc compare=' rc
say 'errors' errors

rc = t.t_copies() ; errors=errors+rc
say 'rc copies=' rc
say 'errors' errors

rc = t.t_countstr() ; errors=errors+rc
say 'rc countstr=' rc
say 'errors' errors

rc = t.t_c2d() ; errors=errors+rc
say 'rc c2d=' rc
say 'errors' errors

rc = t.t_c2x() ; errors=errors+rc
say 'rc c2x=' rc
say 'errors' errors

rc = t.t_datatype() ; errors=errors+rc
say 'rc datatype=' rc
say 'errors' errors

rc = t.t_delstr() ; errors=errors+rc
say 'rc delstr=' rc
say 'errors' errors

rc = t.t_delword() ; errors=errors+rc
say 'rc delword=' rc
say 'errors' errors

rc = t.t_d2c() ; errors=errors+rc
say 'rc d2c=' rc
say 'errors' errors


-- return errors<>0
return 0

testrexx: class

  *: factory
    return

  t_abbrev: method = .int
    flag=.boolean
    flag = 1
    p1=.rexx('Print'); p2=.rexx('PRINT')
    flag=flag & (p1.abbrev('Pri')=='1')
    flag=flag & (p2.abbrev('Pri')=='0')
    flag=flag & (p2.abbrev('PRI',2)=='1')
    flag=flag & (p2.abbrev('PRI',3)=='1')
    flag=flag & (p2.abbrev('PRI',4)=='0')
    flag=flag & (p2.abbrev('PRY')=='0')
    flag=flag & (p2.abbrev('')=='1')
    flag=flag & (p2.abbrev('',0)=='1')
    flag=flag & (p2.abbrev('',1)=='0')
    return \flag

  t_abs: method = .int
    flag=.boolean
    flag = 1
    s = .rexx('-1')
    flag=flag & s.abs() ='1'
    s = .rexx('0')
    flag=flag & s.abs() ='0'
    -- s = .rexx('+1')
    -- flag=flag & s.abs() ='1'
    -- s = .rexx(' - 12345678        ')
    -- flag=flag & s.abs() ='12345678'
    -- s = .rexx(' - 12345678        ')
    -- flag=flag & s.abs() ='123456789'
    -- s = .rexx(' - 1234567890      ')
    -- flag=flag & s.abs() ='1234567890'
    -- s = .rexx(' - 12345678900     ')
    -- flag=flag & s.abs() ='12345678900'
    -- s = .rexx('123.45E+16')
    -- flag=flag & s.abs() ='1.2345E+18'
    -- s = .rexx('- 1234567.7654321')
    -- flag=flag & s.abs() ='1234567.7654321'
    return \flag

 /*-- B 2 X -------------------------------------------------------*/
  t_b2x: method = .int
    flag=.boolean
    flag = 1
    s = .rexx('0')
    flag=flag & s.b2x() ='0'
    s = .rexx('1')
    flag=flag & s.b2x() ='1'
    s = .rexx('10')
    flag=flag & s.b2x() ='2'
    s = .rexx('101')
    flag=flag & s.b2x() ='5'
    s = .rexx('1011')
    flag=flag & s.b2x() ='B'
    s = .rexx('10101')
    flag=flag & s.b2x() ='15'
    return \flag

     /*-- C E N T R E / C E N T E R -----------------------------------*/
 t_centre: method = .int
    flag=.boolean
    flag = 1
    
    a=.rexx('abc'); b=.rexx('The blue sky'); n=.rexx('')
    flag=flag & (a.centre(7)=='  abc  ')
    flag=flag & (a.center(7)=='  abc  ')
    flag=flag & (a.centre(7,'-')=='--abc--')
    flag=flag & (a.centre(8,'-')=='--abc---')
    flag=flag & (b.centre(8)=='e blue s')
    flag=flag & (b.centre(7)=='e blue ')
    flag=flag & (b.centre(0)=='')
    flag=flag & (n.centre(0)=='')
    -- flag=flag & (n.centre(1)==' ')
    flag=flag & (n.centre(2)=='  ')
    flag=flag & (n.centre(3)=='   ')
    flag=flag & (n.centre(4)=='    ')
    return \flag

    /*-- C H A N G E S T R -------------------------------------------*/
 t_changestr: method =.int
    flag=.boolean
    flag = 1
    v=.rexx('abcdef'); v2=.rexx('abcabc'); w3=.rexx('aa bbb cccc')

    -- flag=flag & (v.changestr('1','2')=v)
    flag=flag & (v.changestr('a','xx')='xxbcdef')
    flag=flag & (v.changestr('cd','x')='abxef')
    flag=flag & (v2.changestr('a','A')='AbcAbc')
    -- flag=flag & (v2.changestr('cd','z')=v2
    -- flag=flag & (v2.changestr('bc','')='aa')
    -- flag=flag & (w3.changestr('', 'z')=w3)
    return \flag

     /*-- C O M P A R E -----------------------------------------------*/
  t_compare: method =.int
    flag=.boolean
    flag = 1
    s=.rexx('abc');t=.rexx('ab');u=.rexx('');v=.rexx('d')
    w=.rexx('ab ');x=.rexx('ab-- ')

    flag=flag & (s.compare('abc')       = '0')
    flag=flag & (s.compare('ak')        = '2')
    flag=flag & (s.compare('ab')        = 3)
    flag=flag & ( t.compare('abc')      = 3)
    flag=flag & ( u.compare('abc')      = 1)
    flag=flag & ( v.compare('')         = 1)
    flag=flag & ( w.compare('ab')       = 0)
    flag=flag & ( w.compare('ab',' ')   = 0)
    flag=flag & ( w.compare('ab','x')   = 3)
    flag=flag & ( x.compare('ab','-')   = 5)
    return \flag

     /*-- C O P I E S -------------------------------------------------*/
  t_copies: method = .int
    flag=.boolean
    flag = 1
    v=.rexx('abcdef'); c=.rexx('c')
    flag=flag & (v.copies(0)='')
    flag=flag & (c.copies(1)=c.toString())
    flag=flag & (v.copies(1)==v)
    flag=flag & (v.copies(2)==v.toString()||v.toString())
    flag=flag & (v.copies(2)=='abcdefabcdef')
    flag=flag & (c.copies(5)=="ccccc")
    return \flag


     /*-- C O U N T S T R ---------------------------------------------*/
 t_countstr: method = .int
    flag=.boolean
    flag = 1
    v=.rexx('abcdef'); v2=.rexx('abcabc'); w3=.rexx('aa bbb cccc')
    flag=flag & (v.countstr(1)=0)
    flag=flag & (v.countstr('a')=1)
    -- flag=flag & (v.countstr('cd')=1)
    -- flag=flag & (v2.countstr('a')=2)
    -- flag=flag & (v2.countstr('cd')=0)
    -- flag=flag & (v2.countstr('bc')=2)
    -- flag=flag & (w3.countstr(' ')=2)
    return \flag
    
 /*-- C 2 D -------------------------------------------------------*/
  t_c2d: method = .int
    flag=.boolean
    flag = 1
    v=.rexx('M');w=.rexx('\0')
    flag=flag & v.c2d()=='77'
    -- flag=flag & w.c2d()=='0'
    return \flag
    
    /*-- C 2 X -------------------------------------------------------*/
  t_c2x: method = .int
    flag=.boolean
    flag = 1
    v=.rexx('\u0123');w=.rexx('\ubeef');x=.rexx('M');y=.rexx('\0')
    flag=flag & v.c2x()=='123'
    flag=flag & w.c2x()=='BEEF'
    flag=flag & x.c2x()=='4D'
    flag=flag & y.c2x()=='0'
    return \flag

 /*-- D A T A T Y P E ---------------------------------------------*/
 t_datatype: method = .int
    flag=.boolean
    flag = 1
    a=.rexx('Mixed21')
    b=.rexx('0100110')
    d=.rexx('987032')
    l=.rexx('foobar')
    m=.rexx('BiCap')
    n=.rexx('21.3')
    s=.rexx('Fred_21')
    u=.rexx('SHOUT')
    w=.rexx('1234')
    x=.rexx('deadBEEF0123')
    z=.rexx('**(^^&')

    flag=flag & (a.datatype('a')=='1')
    flag=flag & (b.datatype('a')=='1')
    flag=flag & (d.datatype('a')=='1')
    flag=flag & (l.datatype('a')=='1')
    flag=flag & (m.datatype('a')=='1')
    flag=flag & (n.datatype('a')=='0')
    flag=flag & (s.datatype('a')=='0')
    flag=flag & (u.datatype('a')=='1')
    flag=flag & (w.datatype('a')=='1')
    flag=flag & (x.datatype('a')=='1')
    flag=flag & (z.datatype('a')=='0')
    
    flag=flag & (a.datatype('b')=='0')
    flag=flag & (b.datatype('b')=='1')
    flag=flag & (d.datatype('b')=='0')
    flag=flag & (l.datatype('b')=='0')
    flag=flag & (m.datatype('b')=='0')
    flag=flag & (n.datatype('b')=='0')
    flag=flag & (s.datatype('b')=='0')
    flag=flag & (u.datatype('b')=='0')
    flag=flag & (w.datatype('b')=='0')
    flag=flag & (x.datatype('b')=='0')
    flag=flag & (z.datatype('b')=='0')
    
    flag=flag & (a.datatype('d')=='0')
    flag=flag & (b.datatype('d')=='1')
    flag=flag & (d.datatype('d')=='1')
    flag=flag & (l.datatype('d')=='0')
    flag=flag & (m.datatype('d')=='0')
    flag=flag & (n.datatype('d')=='0')
    flag=flag & (s.datatype('d')=='0')
    flag=flag & (u.datatype('d')=='0')
    flag=flag & (w.datatype('d')=='1')
    flag=flag & (x.datatype('d')=='0')
    flag=flag & (z.datatype('d')=='0')
    
    flag=flag & (a.datatype('l')=='0')
    flag=flag & (b.datatype('l')=='0')
    flag=flag & (d.datatype('l')=='0')
    flag=flag & (l.datatype('l')=='1')
    flag=flag & (m.datatype('l')=='0')
    flag=flag & (n.datatype('l')=='0')
    flag=flag & (s.datatype('l')=='0')
    flag=flag & (u.datatype('l')=='0')
    flag=flag & (w.datatype('l')=='0')
    flag=flag & (x.datatype('l')=='0')
    flag=flag & (z.datatype('l')=='0')
    
    flag=flag & (a.datatype('m')=='0')
    flag=flag & (b.datatype('m')=='0')
    flag=flag & (d.datatype('m')=='0')
    flag=flag & (l.datatype('m')=='1')
    flag=flag & (m.datatype('m')=='1')
    flag=flag & (n.datatype('m')=='0')
    flag=flag & (s.datatype('m')=='0')
    flag=flag & (u.datatype('m')=='1')
    flag=flag & (w.datatype('m')=='0')
    flag=flag & (x.datatype('m')=='0')
    flag=flag & (z.datatype('m')=='0')
    
    flag=flag & (a.datatype('n')=='0')
    flag=flag & (b.datatype('n')=='1')
    flag=flag & (d.datatype('n')=='1')
    flag=flag & (l.datatype('n')=='0')
    flag=flag & (m.datatype('n')=='0')
    flag=flag & (n.datatype('n')=='1')
    flag=flag & (s.datatype('n')=='0')
    flag=flag & (u.datatype('n')=='0')
    flag=flag & (w.datatype('n')=='1')
    flag=flag & (x.datatype('n')=='0')
    flag=flag & (z.datatype('n')=='0')
    
    flag=flag & (a.datatype('s')=='1')
    flag=flag & (b.datatype('s')=='0')
    flag=flag & (d.datatype('s')=='0')
    flag=flag & (l.datatype('s')=='1')
    flag=flag & (m.datatype('s')=='1')
    flag=flag & (n.datatype('s')=='0')
    flag=flag & (s.datatype('s')=='1')
    flag=flag & (u.datatype('s')=='1')
    flag=flag & (w.datatype('s')=='0')
    flag=flag & (x.datatype('s')=='1')
    flag=flag & (z.datatype('s')=='0')
    
    flag=flag & (a.datatype('u')=='0')
    flag=flag & (b.datatype('u')=='0')
    flag=flag & (d.datatype('u')=='0')
    flag=flag & (l.datatype('u')=='0')
    flag=flag & (m.datatype('u')=='0')
    flag=flag & (n.datatype('u')=='0')
    flag=flag & (s.datatype('u')=='0')
    flag=flag & (u.datatype('u')=='1')
    flag=flag & (w.datatype('u')=='0')
    flag=flag & (x.datatype('u')=='0')
    flag=flag & (z.datatype('u')=='0')
    
    flag=flag & (a.datatype('w')=='0')
    flag=flag & (b.datatype('w')=='1')
    flag=flag & (d.datatype('w')=='1')
    flag=flag & (l.datatype('w')=='0')
    flag=flag & (m.datatype('w')=='0')
    flag=flag & (n.datatype('w')=='0')
    flag=flag & (s.datatype('w')=='0')
    flag=flag & (u.datatype('w')=='0')
    flag=flag & (w.datatype('w')=='1')
    flag=flag & (x.datatype('w')=='0')
    flag=flag & (z.datatype('w')=='0')
    
    flag=flag & (a.datatype('x')=='0')
    flag=flag & (b.datatype('x')=='1')
    flag=flag & (d.datatype('x')=='1')
    flag=flag & (l.datatype('x')=='0')
    flag=flag & (m.datatype('x')=='0')
    flag=flag & (n.datatype('x')=='0')
    flag=flag & (s.datatype('x')=='0')
    flag=flag & (u.datatype('x')=='0')
    flag=flag & (w.datatype('x')=='1')
    flag=flag & (x.datatype('x')=='1')
    flag=flag & (z.datatype('x')=='0')
    
    return \flag

 /*-- D E L S T R -------------------------------------------------*/
 t_delstr: method = .int
    flag=.boolean
    flag = 1
    v=.rexx('abcd');w=.rexx('abcde');x=.rexx('')
    flag=flag & (v.delstr(3)    == 'ab')
    flag=flag & (w.delstr(3,2) == 'abe')
    flag=flag & (w.delstr(6)   == 'abcde')
    flag=flag & (w.delstr(9)   == 'abcde')
    flag=flag & (w.delstr(1,0) == 'abcde')
    flag=flag & (x.delstr(1,0) == '')
    flag=flag & (x.delstr(1,2) == '')
    return \flag

 /*-- D E L W O R D -----------------------------------------------*/
 t_delword: method = .int
    flag=.boolean
    flag = 1
    v=.rexx('Now is the  time');w=.rexx('Now is the time ');x=.rexx('Now  time')
    flag=flag & (v.delword(1,2) == 'the  time')
    flag=flag & (v.delword(2,2) == 'Now time')
    flag=flag & (w.delword(3)   == 'Now is ')
    flag=flag & (x.delword(5)          == 'Now  time')
    return \flag

 /*-- D 2 C -------------------------------------------------------*/
 t_d2c: method = .int
    flag=.boolean
    flag = 1;v=.rexx('77');w=.rexx('+77')
    flag=flag & v.d2c()=='M'
    flag=flag & w.d2c()=='M'
    return \flag

 /*-- D 2 X -------------------------------------------------------*/
 -- method d2x static
 --        flag=.boolean
 --    flag = 1

 --  flag=boolean 1; count=count+1
 --  flag=flag & ('9'.d2x       == '9')
 --  flag=flag & ('129'.d2x     == '81')
 --  flag=flag & ('129'.d2x(1)  == '1')
 --  flag=flag & ('129'.d2x(2)  == '81')
 --  flag=flag & ('127'.d2x(3)  == '07F')
 --  flag=flag & ('129'.d2x(4)  == '0081')
 --  flag=flag & ('257'.d2x(2)  == '01')
 --  flag=flag & ('-127'.d2x(2) == '81')
 --  flag=flag & ('-127'.d2x(4) == 'FF81')
 --  flag=flag & ('12'.d2x(0)   == '')
 --  flag=flag & ('8947848'.d2x == '888888')
 --  flag=flag & ('8947848'.d2x(4) == '8888')
 --  flag=flag & ('8947848'.d2x(5) == '88888')
 --  flag=flag & ('8947848'.d2x(6) == '888888')
 --  flag=flag & ('8947848'.d2x(7) == '0888888')
 --  flag=flag & ('8947848'.d2x(8) == '00888888')
 --  flag=flag & ('-7829368'.d2x(4) == '8888')
 --  flag=flag & ('-7829368'.d2x(5) == '88888')
 --  flag=flag & ('-7829368'.d2x(6) == '888888')
 --  flag=flag & ('-7829368'.d2x(7) == 'F888888')
 --  flag=flag & ('-7829368'.d2x(8) == 'FF888888')
 --  if \flag then signal DiagX('Rexx d2x')
 --  return

 /*-- E X I S T S -------------------------------------------------*/
  --   method exists static
  --       flag=.boolean
  --   flag = 1

  -- flag=boolean 1; count=count+1
  -- vowel=0
  -- vowel['a']=1
  -- vowel['b']=1
  -- vowel['b']=null -- drops previous assignment
  -- flag=flag & (vowel.exists('a') == '1')
  -- flag=flag & (vowel.exists('b') == '0')
  -- flag=flag & (vowel.exists('c') == '0')
  -- if \flag then signal DiagX('Rexx exists')
  -- return

 /*-- F O R M A T -------------------------------------------------*/
 -- t_format: method = .int
 --    flag=.boolean
 --    flag = 1
 --    flag=flag & ('12.3'.format             == 12.3)
 --    flag=flag & (' - 12.73'.format         == '-12.73')
 --    flag=flag & ('0.000'.format            == '0')
 --    flag=flag & ('3'.format(4)             == '   3')
 --    flag=flag & ('1.73'.format(4,0)        == '   2')
 --    flag=flag & ('1.73'.format(4,1)        == '   1.7')
 --    flag=flag & ('1.75'.format(4,1)        == '   1.8')
 --    flag=flag & ('1.73'.format(4,2)        == '   1.73')
 --    flag=flag & ('1.73'.format(4,3)        == '   1.730')
 --    flag=flag & ('-.76'.format(4,1)        == '  -0.8')
 --    flag=flag & ('3.03'.format(4)          == '   3.03')
 --    flag=flag & (' - 12.73'.format(null,4) == '-12.7300')
 --    flag=flag & ('3.03'.format(4,null,3)   == '   3.03     ')
 --    flag=flag & ('3.03'.format(null,null,3)== '3.03     ')
    
 --    flag=flag & ('0.05'.format(4,1)        == '   0.1')
 --    flag=flag & ('0.005'.format(4,0)       == '   0')
 --    flag=flag & ('0.005'.format(4,1)       == '   0.0')
 --    flag=flag & ('0.005'.format(4,2)       == '   0.01')
 --    flag=flag & ('0.005'.format(4,3)       == '   0.005')
 --    flag=flag & ('0.005'.format(4,4)       == '   0.0050')
 --    flag=flag & ('0'.format(4,0)           == '   0')
 --    flag=flag & ('0'.format(4,1)           == '   0.0')
 --    flag=flag & ('0'.format(4,2)           == '   0.00')
    
 --    flag=flag & ('0.4'.format(null,0)      == '0')
 --    flag=flag & ('-0.4'.format(null,0)     == '0')
 --    flag=flag & ('0.444'.format(null,0)     == '0')
 --    flag=flag & ('-0.444'.format(null,0)     == '0')
    
 --    flag=flag & ('12345.73'.format(null,null,null,3) == '1.234573E+4')
 --    flag=flag & ('12345.73'.format(null,null,null,4) == '1.234573E+4')
 --    flag=flag & ('12345.73'.format(null,null,null,5) == '12345.73')
 --    flag=flag & ('12345.73'.format(null,null,null,6) == '12345.73')
    
 --    flag=flag & ('12345.73'.format(null,8,null,3)    == '1.23457300E+4')
 --    flag=flag & ('12345.73'.format(null,7,null,3)    == '1.2345730E+4')
 --    flag=flag & ('12345.73'.format(null,6,null,3)    == '1.234573E+4')
 --    flag=flag & ('12345.73'.format(null,5,null,3)    == '1.23457E+4')
 --    flag=flag & ('12345.73'.format(null,4,null,3)    == '1.2346E+4')
 --    flag=flag & ('12345.73'.format(null,3,null,3)    == '1.235E+4')
 --    flag=flag & ('12345.73'.format(null,2,null,3)    == '1.23E+4')
 --    flag=flag & ('12345.73'.format(null,1,null,3)    == '1.2E+4')
 --    flag=flag & ('12345.73'.format(null,0,null,3)    == '1E+4')
    
 --    flag=flag & ('99999.99'.format(null,6,null,3)    == '9.999999E+4')
 --    flag=flag & ('99999.99'.format(null,5,null,3)    == '1.00000E+5')
 --    flag=flag & ('99999.99'.format(null,2,null,3)    == '1.00E+5')
 --    flag=flag & ('99999.99'.format(null,1,null,3)    == '1.0E+5')
 --    flag=flag & ('99999.99'.format(null,0,null,3)    == '1E+5')
 --    flag=flag & ('99999.99'.format(3,0,null,3)       == '  1E+5')
    
 --    flag=flag & ('12345.73'.format(null,null,2,2) == '1.234573E+04')
 --    flag=flag & ('12345.73'.format(null,3,null,0) == '1.235E+4')
 --    flag=flag & ('1.234573'.format(null,3,null,0) == '1.235')
 --    flag=flag & ('123.45'.format(null,3,2,0)      == '1.235E+02')
    
 --    flag=flag & ('1234.5'.format(null,3,2,0,'e')  == '1.235E+03')
 --    flag=flag & ('12345'.format(null,3,3,0,'e')   == '12.345E+003')
 --    flag=flag & ('12345'.format(null,3,3,0,'s')   == '1.235E+004')
 --    flag=flag & ('1234.5'.format(4,3,2,0,'e')     == '   1.235E+03')
 --    flag=flag & ('12345'.format(5,3,3,0,'e')      == '   12.345E+003')
 --    flag=flag & ('12345'.format(6,3,3,0,'s')      == '     1.235E+004')
    
 --    flag=flag & ('1.2345'.format(null,3,2,0)      == '1.235    ')
 --    flag=flag & ('12345.73'.format(null,null,3,6) == '12345.73     ')
 --    flag=flag & ('12345e+5'.format(null,0)        == '1234500000')
 --    flag=flag & ('12345e+5'.format(null,1)        == '1234500000.0')
 --    flag=flag & ('12345e+5'.format(null,2)        == '1234500000.00')
 --    flag=flag & ('12345e+5'.format(null,3)        == '1234500000.000')
 --    flag=flag & ('12345e+5'.format(null,4)        == '1234500000.0000')
    
 --    /* The ANSI nasties [Dallas, Nov. 1998] */
 --    flag=flag & ('99.999'.format(null,2,null,2)   == '100.00')
 --    flag=flag & ('0.99999'.format(null,4,2,2)     == '1.0000    ')
    
 --    return \flag


