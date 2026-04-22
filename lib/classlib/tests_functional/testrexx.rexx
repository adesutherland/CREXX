options levelb comments_dash
namespace testrexx expose testrexx
import rexx

say 'started testrexx'
errors = 0

t = .testrexx()

rc = t.t_abbrev() ; errors=errors+rc
say 'rc' rc
say 'errors' errors

rc = t.t_abs() ; errors=errors+rc
say 'rc' rc
say 'errors' errors

rc = t.t_b2x() ; errors=errors+rc
say 'rc' rc
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
    s = .rexx('+1')
    flag=flag & s.abs() ='1'
    s = .rexx(' - 12345678        ')
    flag=flag & s.abs() ='12345678'
    s = .rexx(' - 12345678        ')
    flag=flag & s.abs() ='123456789'
    s = .rexx(' - 1234567890      ')
    flag=flag & s.abs() ='1234567890'
    s = .rexx(' - 12345678900     ')
    flag=flag & s.abs() ='12345678900'
    s = .rexx('123.45E+16')
    flag=flag & s.abs() ='1.2345E+18'
    s = .rexx('- 1234567.7654321')
    flag=flag & s.abs() ='1234567.7654321'
    return \flag

 /*-- B 2 X -------------------------------------------------------*/
  t_b2x: method = .int
    flag=.boolean
    flag = 1
    s = .rexx('0')
    flag=flag & s.b2x() ='0'
    say s.toString() s.b2x()
    s = .rexx('1')
    flag=flag & s.b2x() ='1'
    say s.toString()  s.b2x()
    s = .rexx('10')
    flag=flag & s.b2x() ='2'
    say s.toString() s.b2x()
    s = .rexx('101')
    flag=flag & s.b2x() ='5'
    say s.toString() s.b2x()
    s = .rexx('1011')
    flag=flag & s.b2x() ='B'
    say s.toString() s.b2x()
    s = .rexx('10101')
    flag=flag & s.b2x() ='15'
    say s.toString() s.b2x()
    return \flag
