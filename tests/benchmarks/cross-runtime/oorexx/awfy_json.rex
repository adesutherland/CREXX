/*
 * Full Are We Fast Yet? Json workload through ooRexx's supplied json.cls.
 * The fixture is the exact minified RAP payload from upstream commit
 * 74306fec151070fd07157cefeacf19e7e0bcdc89. This is a disclosed standard-
 * library DOM adaptation, not an implementation of AWFY's minimal parser.
 */
parse arg repetitions fixturePath
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'
if fixturePath='' then call fail 'fixture path is required'

fixtureBytes=chars(fixturePath)
if fixtureBytes<>25820 then -
  call fail 'expected a 25820-byte fixture, got' fixtureBytes
payload=charin(fixturePath,1,fixtureBytes)
call charin fixturePath,1,0

operationsCount=0
do iteration=1 to repetitions
  document=.json~fromJson(payload)
  if \document~isA(.directory) then call fail 'document root is not an object'

  head=document['head']
  if \head~isA(.directory) then call fail 'head is not an object'

  operations=document['operations']
  if \operations~isA(.array) then call fail 'operations is not an array'
  operationsCount=operations~items
  if operationsCount<>156 then -
    call fail 'expected 156 operations, got' operationsCount
end

say 'benchmark=awfy_json repetitions='repetitions -
  'fixture_bytes='fixtureBytes 'operations='operationsCount -
  'adaptation=standard-library-dom'
say 'PASS: Full AWFY Json ooRexx standard-library port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::requires 'json.cls'
