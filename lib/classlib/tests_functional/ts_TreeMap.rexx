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

return errors<>0


