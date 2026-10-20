/* rexx-csv Classic consumer. MIT License; see ../LICENSE.
 * Run in a directory containing rexx_csv.rexx.
 * Optional single command string: '"input file.csv" "output file.csv"'
 * CHARIN/CHAROUT preserve bytes; PARSE validates UTF-8 before CSV decoding.
 */
numeric digits 20
parse arg paths
paths = strip(paths)
names.0 = 0
do while paths \== ''
  names.0 = names.0 + 1
  n = names.0
  if n > 2 then do
    say 'Expected at most two paths, each enclosed in double quotes'
    exit 1
  end
  if left(paths, 1) \== '"' then do
    say 'Enclose each path in double quotes inside the command string'
    exit 1
  end
  close = pos('"', paths, 2)
  if close = 0 then do
    say 'Missing closing quote in path'
    exit 1
  end
  names.n = substr(paths, 2, close - 2)
  if names.n == '' then exit 1
  paths = strip(substr(paths, close + 1))
end
text = 'name,city' || '0a'x || '"Ada, Jr.",' || '4d6f6e7472c3a9616c'x || '0a'x
if names.0 > 0 then do
  input = names.1
  if stream(input, 'C', 'OPEN READ') \== 'READY:' then do
    say 'Cannot open input file:' input
    exit 1
  end
  size = stream(input, 'C', 'QUERY SIZE')
  if datatype(size, 'W') = 0 then do
    say 'Input must be a regular file with a known byte size'
    call stream input, 'C', 'CLOSE'
    exit 1
  end
  if size > 16777216 then do
    say 'E_LIMIT input_bytes'
    call stream input, 'C', 'CLOSE'
    exit 1
  end
  text = charin(input, 1, size)
  call stream input, 'C', 'CLOSE'
  if length(text) \= size then do
    say 'Could not read the complete input file'
    exit 1
  end
end
document = 'rexx_csv'('PARSE', text)
if 'rexx_csv'('OK', document) = 0 then do
  say 'rexx_csv'('ERROR_CODE', document) 'record' 'rexx_csv'('ERROR_RECORD', document) 'field' 'rexx_csv'('ERROR_FIELD', document)
  exit 1
end
/* WITH_FIELD demonstrates editing without changing document:
 * changed = 'rexx_csv'('WITH_FIELD', document, 2, 1, 'Ada')
 * output = 'rexx_csv'('WRITE', changed)
 */
output = 'rexx_csv'('WRITE', document)
if 'rexx_csv'('OK', output) = 0 then do
  say 'rexx_csv'('ERROR', output)
  exit 1
end
if names.0 > 1 then do
  target = names.2
  /* Only open/replace the output after all decoding and CSV checks succeed. */
  if stream(target, 'C', 'OPEN WRITE REPLACE') \== 'READY:' then do
    say 'Cannot open output file:' target
    exit 1
  end
  remaining = charout(target, 'rexx_csv'('TEXT', output), 1)
  call stream target, 'C', 'CLOSE'
  if remaining \= 0 then do
    say 'Could not write the complete output file'
    exit 1
  end
end
say 'PASS: CSV example;' 'rexx_csv'('ROW_COUNT', document) 'rows'
exit 0
