options levelb
import data_HashMap

a = .HashMap()

rc = a.put('aap','noot')
rc = a.put('noot','mies')
rc = a.put('mies','wim')
rc = a.put('wim','zus')
rc = a.put('zus','jet')
rc = a.put('jet',1)
rc = a.put('teun',2)
rc = a.put('vuur',3)
rc = a.put('gijs',4)
rc = a.put('lam',5)
rc = a.put('kees',6)
rc = a.put('bok',7)
rc = a.put('weide',8)
rc = a.put('does',9)
rc = a.put('hok',10)
rc = a.put('duif',11)
rc = a.put('schapen',12)

say a.size()

i = a.iterator()

loop while i.hasNext()
  say i.next()
end

say a.toString()

say 'keyStem() on HashMap'

keystem = a.keyStem()

say 'looping over stem result'
loop l=1 to keystem.0
  say keystem.l
end

say 'number of elements in stem:' keystem.0
