options levelb comments_dash
import rexx

t = .seqerr()

say t.t_compare()

seqerr: class
  *: factory
    return

  t_compare: method = .int
    flag=.boolean
    flag = 1
    s=.rexx('abc');t=.rexx('ab');u=.rexx('');v=.rexx('d')
    w=.rexx('ab ');x=.rexx('ab-- ')
    flag=flag & (s.compare('abc')      == 0)
    flag=flag & (s.compare('ak')       == 2)
    flag=flag & (s.compare('ab')       == 3)
    flag=flag & ( t.compare('abc')      == 3)
    flag=flag & ( u.compare('abc')      == 01)
    flag=flag & ( v.compare('')         == 1)
    flag=flag & ( w.compare('ab')       == 0)
    flag=flag & ( w.compare('ab',' ')   == 0)
    flag=flag & ( w.compare('ab','x')   == 3)
    flag=flag & ( x.compare('ab','-') == 5)
    return \flag
