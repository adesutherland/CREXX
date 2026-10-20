/* Shared L001-L010 and Classic B001-B003/B005 contracts. MIT License. */
parse arg section
if section = '' then section = 'all'
if wordpos(section, 'all ownership fields counts large quotes') = 0 then exit 2
numeric digits 4
if section = 'all' | section = 'ownership' then do
call progress 'START: ownership'
caller.1 = 'retained'
cells.1 = 'private caller cell'
text = '0012,' || 'c3a9f09f9982'x || ',"a' || '0d0a'x || 'b",'
one = 'rexx_csv'('parse', text)
call good 'B001 first', one
call check 'B001 numeric context', digits(), '4'
call check 'B001 external scope', caller.1, 'retained'
call check 'B001 same-named stem', cells.1, 'private caller cell'
numeric digits 20
bad = 'rexx_csv'('PARSE', '"unfinished')
call error 'B001 failure', bad, 'E_UNCLOSED_QUOTE', 1, 1, ''
text = 'changed'
two = 'rexx_csv'('PARSE', 'later')
call check 'B001 retained', get(one, 1, 1), '0012'
call check 'B001 UTF-8', get(one, 1, 2), 'c3a9f09f9982'x
call check 'B001 embedded CRLF', get(one, 1, 3), 'a' || '0d0a'x || 'b'
call check 'B001 second', get(two, 1, 1), 'later'
changed = 'rexx_csv'('WITH_FIELD', one, 1, 1, 'Ada')
call good 'B002 replacement', changed
call check 'B002 original', get(one, 1, 1), '0012'
call check 'B002 edited', get(changed, 1, 1), 'Ada'
call check 'B002 width', 'rexx_csv'('FIELD_COUNT', changed, 1), '4'
call check 'B002 unchanged field', get(changed, 1, 3), 'a' || '0d0a'x || 'b'
call check 'B002 empty field', get(changed, 1, 4), ''
field = 'original'
row = 'rexx_csv'('ROW', field)
written = 'rexx_csv'('WRITE', row)
field = 'mutated'
row = 'rexx_csv'('ROW', field)
call check 'B002 written snapshot', 'rexx_csv'('TEXT', written), 'original' || '0d0a'x
nested = 'rexx_csv'('PARSE', 'rexx_csv'('TEXT', 'rexx_csv'('WRITE', one)))
call check 'B003 nested', get(nested, 1, 2), 'c3a9f09f9982'x
combined = 'rexx_csv'('TABLE', one, two, 'rexx_csv'('TABLE'))
call check 'B003 combined rows', 'rexx_csv'('ROW_COUNT', combined), '2'
call check 'B003 combined first', get(combined, 1, 1), '0012'
call check 'B003 combined last', get(combined, 2, 1), 'later'
call check 'B002 failed rows', 'rexx_csv'('ROW_COUNT', bad), '0'
call check 'B002 failed field count', 'rexx_csv'('FIELD_COUNT', bad, 1), '-1'
call check 'B002 invalid row count', 'rexx_csv'('ROW_COUNT', 'wrong'), '-1'
call check 'B002 invalid opaque', 'rexx_csv'('OK', 'RXCSV01'), '0'
call check 'B002 invalid text', 'rexx_csv'('TEXT', one), ''
call check 'B002 invalid diagnostics', 'rexx_csv'('ERROR_CODE', 'wrong'), 'E_ARGUMENT'
call check 'B002 negative row', 'rexx_csv'('FIELD_COUNT', one, -1), '-1'
call check 'B002 fractional row', 'rexx_csv'('FIELD_COUNT', one, 1.5), '-1'
call check 'B002 nonnumeric row', 'rexx_csv'('FIELD_COUNT', one, 'x'), '-1'
call error 'B002 bad field', 'rexx_csv'('FIELD', one, 1, 5), 'E_ARGUMENT', 0, 0, ''
call error 'B002 bad field row', 'rexx_csv'('FIELD', one, 'x', 1), 'E_ARGUMENT', 0, 0, ''
call error 'B002 bad edit', 'rexx_csv'('WITH_FIELD', one, 0, 1, 'x'), 'E_ARGUMENT', 0, 0, ''
call error 'B002 bad table', 'rexx_csv'('TABLE', one, 'x'), 'E_ARGUMENT', 0, 0, ''
call error 'B002 bad writer', 'rexx_csv'('WRITE', bad), 'E_ARGUMENT', 0, 0, ''
call error 'B002 unknown operation', 'rexx_csv'('UNKNOWN'), 'E_ARGUMENT', 0, 0, ''
call error 'B002 argument count', 'rexx_csv'('PARSE'), 'E_ARGUMENT', 0, 0, ''
call error 'B002 omitted parse text', 'rexx_csv'('PARSE',), 'E_ARGUMENT', 0, 0, ''
call error 'B002 omitted row field', 'rexx_csv'('ROW',, 'x'), 'E_ARGUMENT', 0, 0, ''
call error 'B002 omitted edit text', 'rexx_csv'('WITH_FIELD', one, 1, 1,), 'E_ARGUMENT', 0, 0, ''
/* Strict UTF-8 rejects continuation/overlong/surrogate/truncated/out-of-range. */
invalid = '80 c0af c1bf c2 e08080 eda080 f0808080 f4908080 f5808080 f09f99 e228a1 ff'
do i = 1 to words(invalid)
  bytes = x2c(word(invalid, i))
  call error 'B005 parse', 'rexx_csv'('PARSE', bytes), 'E_ARGUMENT', 0, 0, ''
  call error 'B005 write', 'rexx_csv'('WRITE', 'rexx_csv'('ROW', bytes)), 'E_ARGUMENT', 0, 0, ''
end
scalars = '00 7f c280 dfbf e0a080 ed9fbf ee8080 efbfbf f0908080 f48fbfbf'
do i = 2 to words(scalars)
  bytes = x2c(word(scalars, i))
  call check 'B005 valid scalar', get('rexx_csv'('PARSE', bytes), 1, 1), bytes
end
say 'PASS: B001-B003/B005 ownership, recovery, API errors and UTF-8 boundary'

end
numeric digits 20
ascii = copies('a', 1048576)
unicode = copies('c3a9'x, 524288)

if section = 'all' | section = 'fields' then do
call progress 'START: fields'
/* Inclusive decoded-byte bounds, independently checked in bytes. */
call check 'L002 byte count', length(unicode), '1048576'
do which = 1 to 2
  if which = 1 then text = ascii
  else text = unicode
  call progress 'START: fields variant' which 'parse accepted'
  accepted = 'rexx_csv'('PARSE', text)
  call good 'L001/L002 accepted', accepted
  call check 'L001/L002 field', get(accepted, 1, 1), text
  call progress 'START: fields variant' which 'parse excess'
  call error 'L001/L002 excess', 'rexx_csv'('PARSE', text || 'a'), 'E_LIMIT', 1, 1, 'field_bytes'
  call progress 'START: fields variant' which 'write accepted'
  written = 'rexx_csv'('WRITE', 'rexx_csv'('ROW', text))
  call good 'L007 field accepted', written
  call check 'L007 exact bytes', 'rexx_csv'('TEXT', written), text || '0d0a'x
  call progress 'START: fields variant' which 'write excess'
  call error 'L007 field excess', 'rexx_csv'('WRITE', 'rexx_csv'('ROW', text || 'a')), 'E_LIMIT', 1, 1, 'field_bytes'
end
/* NUL at the first excess position wins; an earlier excess wins over later NUL. */
call error 'NUL before size', 'rexx_csv'('PARSE', ascii || '00'x), 'E_NUL', 1, 1, ''
call error 'size before NUL', 'rexx_csv'('PARSE', ascii || 'a' || '00'x), 'E_LIMIT', 1, 1, 'field_bytes'
call error 'writer NUL before size', 'rexx_csv'('WRITE', 'rexx_csv'('ROW', ascii || '00'x)), 'E_NUL', 1, 1, ''
call error 'writer size before NUL', 'rexx_csv'('WRITE', 'rexx_csv'('ROW', ascii || 'a' || '00'x)), 'E_LIMIT', 1, 1, 'field_bytes'
say 'PASS: L001/L002/L007 decoded ASCII/UTF-8 field limits and error order'

end

if section = 'all' | section = 'counts' then do
call progress 'START: counts'
text = copies('a' || '0a'x, 10000)
accepted = 'rexx_csv'('PARSE', text)
call good 'L003 accepted', accepted
call check 'L003 rows', 'rexx_csv'('ROW_COUNT', accepted), '10000'
call check 'L003 last field', get(accepted, 10000, 1), 'a'
call error 'L003 excess', 'rexx_csv'('PARSE', text || 'a' || '0a'x), 'E_LIMIT', 10001, 1, 'records'
rows = repeated('rexx_csv'('ROW', 'a'), 10000)
call good 'L007 records accepted', 'rexx_csv'('WRITE', rows)
rows = 'rexx_csv'('TABLE', rows, 'rexx_csv'('ROW', 'a'))
call error 'L007 records excess', 'rexx_csv'('WRITE', rows), 'E_LIMIT', 10001, 1, 'records'
text = copies(',', 1023)
accepted = 'rexx_csv'('PARSE', text)
call good 'L004 accepted', accepted
call check 'L004 width', 'rexx_csv'('FIELD_COUNT', accepted, 1), '1024'
call check 'L004 last empty', get(accepted, 1, 1024), ''
call error 'L004 excess', 'rexx_csv'('PARSE', text || ','), 'E_LIMIT', 1, 1025, 'fields_per_record'
/* The adapter supplies a separate 1,024/1,025-field ROW constructor check. */
text = copies(copies(',', 99) || '0a'x, 1000)
accepted = 'rexx_csv'('PARSE', text)
call good 'L005 accepted', accepted
call check 'L005 rows', 'rexx_csv'('ROW_COUNT', accepted), '1000'
do r = 1 to 1000
  if r // 100 = 0 then call progress 'START: count row' r
  call check 'L005 indexed width', 'rexx_csv'('FIELD_COUNT', accepted, r), '100'
  call check 'L005 first indexed field', get(accepted, r, 1), ''
  call check 'L005 last indexed field', get(accepted, r, 100), ''
end
written = 'rexx_csv'('WRITE', accepted)
call good 'L005 every cell through indexed writer', written
expected = copies(copies('"",', 99) || '""' || '0d0a'x, 1000)
call check 'L005 all 100,000 indexed cells', 'rexx_csv'('TEXT', written), expected
call error 'L005 excess', 'rexx_csv'('PARSE', text || '0a'x), 'E_LIMIT', 1001, 1, 'total_fields'
wide = 'rexx_csv'('ROW', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '')
rows = repeated(wide, 1000)
written = 'rexx_csv'('WRITE', rows)
call good 'L007 total accepted', written
call check 'L007 all independent writer fields', 'rexx_csv'('TEXT', written), expected
rows = 'rexx_csv'('TABLE', rows, 'rexx_csv'('ROW', ''))
call error 'L007 total excess', 'rexx_csv'('WRITE', rows), 'E_LIMIT', 1001, 1, 'total_fields'
say 'PASS: L003-L005/L007 count boundaries; every row endpoint; all 100,000 cells via indexed writer'

end

if section = 'all' | section = 'large' then do
call progress 'START: large'
/* 16 MiB input; every field checked against independent construction. */
field = copies('a', 1048573)
text = copies('"' || field || '",', 15) || '"' || field || '"' || '0a'x
call check 'L006 input bytes', length(text), '16777216'
call progress 'START: 16 MiB parse'
accepted = 'rexx_csv'('PARSE', text)
call good 'L006 accepted', accepted
call check 'L006 width', 'rexx_csv'('FIELD_COUNT', accepted, 1), '16'
do c = 1 to 16
  call check 'L006 field', get(accepted, 1, c), field
end
call error 'L006 excess', 'rexx_csv'('PARSE', left(text, length(text) - 1) || '0d0a'x), 'E_LIMIT', 0, 0, 'input_bytes'
call error 'L010 preflight', 'rexx_csv'('PARSE', 'x"' || copies('a', 16777215)), 'E_LIMIT', 0, 0, 'input_bytes'
/* Constructor grows only a 16-field row here, never a whole document loop. */
row = 'rexx_csv'('ROW', ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, ascii, copies('a', 1048559))
call progress 'START: 16 MiB write'
written = 'rexx_csv'('WRITE', row)
call good 'L008 accepted', written
call check 'L008 output bytes', length('rexx_csv'('TEXT', written)), '16777216'
again = 'rexx_csv'('PARSE', 'rexx_csv'('TEXT', written))
call good 'L008 read back', again
do c = 1 to 15
  call check 'L008 round trip', get(again, 1, c), ascii
end
call check 'L008 last', get(again, 1, 16), copies('a', 1048559)
row = 'rexx_csv'('WITH_FIELD', row, 1, 16, copies('a', 1048560))
call error 'L008 excess', 'rexx_csv'('WRITE', row), 'E_LIMIT', 1, 16, 'output_bytes'
row = 'rexx_csv'('WITH_FIELD', row, 1, 16, ascii)
call good '16 MiB owned constructor payload', row
call check '16 MiB payload first', get(row, 1, 1), ascii
call check '16 MiB payload last', get(row, 1, 16), ascii
call error '16 MiB payload output framing', 'rexx_csv'('WRITE', row), 'E_LIMIT', 1, 16, 'output_bytes'
say 'PASS: L006/L008/L010 16 MiB input/output, preflight and every round-trip field'

end

if section = 'all' | section = 'quotes' then do
call progress 'START: quotes'
quotes = copies('"', 1048576)
last = copies('"', 1048562) || 'aaa'
row = 'rexx_csv'('ROW', quotes, quotes, quotes, quotes, quotes, quotes, quotes, last)
call progress 'START: quote expansion write'
written = 'rexx_csv'('WRITE', row)
call good 'L009 quote output', written
call check 'L009 bytes', length('rexx_csv'('TEXT', written)), '16777216'
call progress 'START: quote expansion parse'
again = 'rexx_csv'('PARSE', 'rexx_csv'('TEXT', written))
call good 'L009 read back', again
do c = 1 to 7
  call check 'L009 quoted round trip', get(again, 1, c), quotes
end
call check 'L009 last round trip', get(again, 1, 8), last
row = 'rexx_csv'('WITH_FIELD', row, 1, 8, last || 'a')
call error 'L009 output excess', 'rexx_csv'('WRITE', row), 'E_LIMIT', 1, 8, 'output_bytes'
crlf = copies('0d0a'x, 524288)
written = 'rexx_csv'('WRITE', 'rexx_csv'('ROW', crlf))
call good 'L009 CRLF accepted', written
call check 'L009 CRLF bytes', length('rexx_csv'('TEXT', written)), '1048580'
call check 'L009 CRLF exact', 'rexx_csv'('TEXT', written), '"' || crlf || '"' || '0d0a'x
call error 'L009 CRLF excess', 'rexx_csv'('WRITE', 'rexx_csv'('ROW', crlf || 'a')), 'E_LIMIT', 1, 1, 'field_bytes'
call check 'L009 quoted field accepted', get('rexx_csv'('PARSE', '"' || crlf || '"'), 1, 1), crlf
call error 'L009 quoted field excess', 'rexx_csv'('PARSE', '"' || crlf || 'a"'), 'E_LIMIT', 1, 1, 'field_bytes'
call error 'L009 doubled quote decoded excess', 'rexx_csv'('PARSE', '"' || copies('"', 2097154) || '"'), 'E_LIMIT', 1, 1, 'field_bytes'
say 'PASS: L009 doubled-quote expansion and embedded CRLF boundaries'
end
exit 0

progress: procedure
  parse arg message
  say message
  call stream 'stdout', 'C', 'FLUSH'
  return

check: procedure
  parse arg label, actual, expected
  if actual \== expected then do
    say 'FAIL' label 'expected length' length(expected) 'got length' length(actual)
    exit 1
  end
  return

good: procedure
  parse arg label, value
  call check label || ' status', 'rexx_csv'('OK', value), '1'
  return

error: procedure
  parse arg label, value, code, record, column, limit
  call check label || ' status', 'rexx_csv'('OK', value), '0'
  call check label || ' code', 'rexx_csv'('ERROR_CODE', value), code
  call check label || ' record', 'rexx_csv'('ERROR_RECORD', value), record
  call check label || ' field', 'rexx_csv'('ERROR_FIELD', value), column
  call check label || ' limit', 'rexx_csv'('ERROR_LIMIT', value), limit
  call check label || ' atomic text', 'rexx_csv'('TEXT', value), ''
  if code \== 'E_ARGUMENT' then do
    call check label || ' message', length('rexx_csv'('ERROR', value)) > 0, '1'
  end
  return

get: procedure
  parse arg value, row, column
  field = 'rexx_csv'('FIELD', value, row, column)
  call good 'checked FIELD', field
  return 'rexx_csv'('TEXT', field)

/* Balanced TABLE merging, with bounded source argument lists. */
repeated: procedure
  parse arg row, count
  do i = 1 to count
    chunks.i = row
  end
  do while count > 1
    output = 0
    do input = 1 to count by 2
      output = output + 1
      next = input + 1
      if next <= count then chunks.output = 'rexx_csv'('TABLE', chunks.input, chunks.next)
      else chunks.output = chunks.input
      if input > output then drop chunks.input
      if next <= count then drop chunks.next
    end
    count = output
  end
  return chunks.1
