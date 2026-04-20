options levelb comments_dash
namespace testrexx expose testrexx
import rexx

say 'started testrexx'
errors = 0

t = .testrexx()

rc = t.t_abbrev() ; errors=errors+rc
say 'rc' rc
say 'errors' errors

-- rc = t.t_abs() ; errors=errors+rc
-- say 'rc' rc
-- say 'errors' errors



return errors<>0

testrexx: class

  *: factory
    return

  t_abbrev: method = .int
    flag=.boolean
    flag = 1
    p1=.rexx('Print'); p2=.rexx('PRINT')
    flag=flag & (p1.abbrev('Pri')='1')
    flag=flag & (p2.abbrev('Pri')='0')
    flag=flag & (p2.abbrev('PRI',2)='1')
    flag=flag & (p2.abbrev('PRI',3)='1')
    flag=flag & (p2.abbrev('PRI',4)='0')
    flag=flag & (p2.abbrev('PRY')='0')
    flag=flag & (p2.abbrev('')='1')
    flag=flag & (p2.abbrev('',0)='1')
    flag=flag & (p2.abbrev('',1)='0')
    return \flag

  -- t_abs: method = .int
  --   flag=.boolean
  --   flag = 1
  --   flag=flag & '-1'.abs()                  ='1'
  --   flag=flag &  '0'.abs()                  ='0'
  --   flag=flag & '+1'.abs()                  ='1'
  --   flag=flag & ' - 12345678        '.abs() ='12345678'
  --   flag=flag & ' - 123456789       '.abs() ='123456789'
  --   flag=flag & ' - 1234567890      '.abs() ='1234567890'
  --   flag=flag & ' - 12345678900     '.abs() ='12345678900'
  --   flag=flag & '123.45E+16'.abs()          ='1.2345E+18'
  --   flag=flag & '- 1234567.7654321'.abs()   ='1234567.7654321'
  --   return \flag

