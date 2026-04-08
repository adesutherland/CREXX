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

if a.get('aap') <> 'noot' then do
  errors = errors + 1
end

if a.size() <> 1 then do
  errors = errors + 1
end

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

if a.get('aap') <> 'mies' then do
  errors = errors + 1
end

if a.containsKey('aap') <> 1 then do
  errors = errors + 1
end

if a.firstKey() <> 'aap' then do
  errors = errors + 1
end

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

if a.lastKey() <> 'zus' then do
  errors = errors + 1
end

return errors<>0


