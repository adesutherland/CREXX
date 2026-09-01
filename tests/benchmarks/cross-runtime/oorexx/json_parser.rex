/*
 * Deterministic JSON parser workload using the ooRexx JSON class.
 * The RAP-style payload matches the cREXX and NetRexx ports.
 */
parse arg repetitions
if repetitions='' then repetitions=1
if repetitions<1 then call fail 'repetitions must be positive'

payload='{"head":{"requestCounter":4,"ok":true},"operations":[' ||,
  '["destroy","w54"],' ||,
  '["set","w2",{"activeControl":"w99"}],' ||,
  '["set","w21",{"customVariant":"variant_navigation"}],' ||,
  '["create","w95","rwt.widgets.Composite",{"parent":"w53","bounds":[0,0,1008,586],"children":["w96","w97"],"tabIndex":-1}],' ||,
  '["create","w96","rwt.widgets.Label",{"parent":"w95","bounds":[10,30,112,26],"text":"TableViewer"}],' ||,
  '["listen","w98",{"KeyDown":true,"Modify":true}],' ||,
  '["set","w99",{"itemCount":118,"headerVisible":true,"selection":["w108"]}],' ||,
  '["call","rwt.client.BrowserNavigation","addToHistory",{"entries":[["tableviewer","TableViewer"]]}]' ||,
  ']}'

result=0
do iteration=1 to repetitions
  document=.json~fromJson(payload)
  if \document~isA(.directory) then call fail 'expected root object'
  operations=document['operations']
  result=operations~items
  if result<>8 then call fail 'expected 8 operations, got' result
end

say 'benchmark=json_parser repetitions='repetitions 'operations='result
say 'PASS: JSON Parser ooRexx port'
exit 0

fail: procedure
  parse arg message
  say 'FAIL:' message
  exit 1

::requires 'json.cls'
