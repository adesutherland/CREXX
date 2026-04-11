options levelb
import data_TreeMap

errors = 0

a = .TreeMap()
if a.val = 0 then do
  errors = errors + 1
end

rc = a.put('aap','noot')
if rc <> 0 then  do
  errors = errors + 1
end

say a.get('aap') 

say a.size()

/**
* same key, same value, rc=4
*/
rc = a.put('aap','noot')
if rc <> 4 then  do
  errors = errors + 1
end

/**
* this key is already in, change the value, rc=4
*/
rc = a.put('aap','mies')
if rc <> 0 then  do
  errors = errors + 1
end

say a.containsKey('aap')

say a.firstKey()

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

say a.lastKey()

it = .TreeMapIterator
it = a.iterator()

loop while it.hasNext()
  say it.next()
end

say '|'a.get('missing')'|'

say a.toString()

say a.remove('aap')

say a.remove('zzy')

say 'keyStem() on TreeMap'

keystem = a.keyStem()

say 'looping over stem result'
loop l=1 to keystem.0
  say keystem.l
end

say 'number of elements in stem:' keystem.0
