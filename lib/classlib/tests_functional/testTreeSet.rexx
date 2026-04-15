options levelb
import data_TreeSet

a = .TreeSet()

rc = a.add('aap')
rc = a.add('noot')
rc = a.add('mies')
rc = a.add('wim')
rc = a.add('zus')
rc = a.add('jet')
rc = a.add('teun')
rc = a.add('vuur')
rc = a.add('gijs')
rc = a.add('lam')
rc = a.add('kees')
rc = a.add('bok')
rc = a.add('weide')
rc = a.add('does')
rc = a.add('hok')
rc = a.add('duif')
rc = a.add('schapen')

say a.size()

i = .TreeSetIterator
i = a.iterator()

loop while i.hasNext()
  say i.next()
end

say a.toString()

say 'toArray() on TreeSet'

keystem = a.toArray()

say 'looping over stem result'
loop l=1 to keystem.0
  say keystem.l
end

say 'number of elements in stem:' keystem.0

