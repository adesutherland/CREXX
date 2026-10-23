/* rexx-csv Classic binding. MIT License; see ../LICENSE.
 * Copyright (c) 2026 Adrian Sutherland.
 * Shared CSV contract: ../SPEC.md. No caller stems or global state are used.
 * Opaque values are private indexed strings, not a public serialized format.
 */
numeric digits 20
operation = translate(strip(arg(1)))
select
  when operation = 'PARSE' then do
    if arg() \= 2 | arg(2, 'O') then return misuse()
    return parse_csv(arg(2))
  end
  when operation = 'WRITE' then do
    if arg() \= 2 then return misuse()
    value = arg(2)
    return write_csv()
  end
  when operation = 'ROW' then do
    count = arg() - 1
    do i = 1 to count
      if arg(i + 1, 'O') then return misuse()
      cells.i = arg(i + 1)
    end
    ends.1 = count
    return assemble('R', 1, count)
  end
  when operation = 'TABLE' then do
    records = 0
    fields = 0
    do operand = 2 to arg()
      value = arg(operand)
      if table_valid() = 0 then return misuse()
      nr = c2d(substr(value, 58, 4))
      nf = c2d(substr(value, 62, 4))
      previous = 0
      do row = 1 to nr
        boundary = c2d(substr(value, 66 + 4 * (row - 1), 4))
        if boundary < previous | boundary > nf then return misuse()
        records = records + 1
        ends.records = fields + boundary
        previous = boundary
      end
      do cell = 1 to nf
        if cell_valid(cell) = 0 then return misuse()
        fields = fields + 1
        cells.fields = cell_text(cell)
      end
    end
    return assemble('T', records, fields)
  end
  when operation = 'WITH_FIELD' then do
    if arg() \= 5 | arg(5, 'O') then return misuse()
    value = arg(2)
    flat = locate(arg(3), arg(4))
    if flat = 0 then return misuse()
    records = c2d(substr(value, 58, 4))
    fields = c2d(substr(value, 62, 4))
    previous = 0
    do row = 1 to records
      ends.row = c2d(substr(value, 66 + 4 * (row - 1), 4))
      if ends.row < previous | ends.row > fields then return misuse()
      previous = ends.row
    end
    do cell = 1 to fields
      if cell = flat then cells.cell = arg(5)
      else do
        if cell_valid(cell) = 0 then return misuse()
        cells.cell = cell_text(cell)
      end
    end
    return assemble('T', records, fields)
  end
  when operation = 'FIELD' then do
    if arg() \= 4 then return misuse()
    value = arg(2)
    flat = locate(arg(3), arg(4))
    if flat = 0 then return misuse()
    return header('F', 1, '', 0, 0, '') || cell_text(flat)
  end
  when operation = 'FIELD_COUNT' then do
    if arg() \= 3 then return -1
    value = arg(2)
    row = arg(3)
    if table_valid() = 0 | whole(row) = 0 then return -1
    nr = c2d(substr(value, 58, 4))
    nf = c2d(substr(value, 62, 4))
    if row < 1 | row > nr then return -1
    previous = 0
    if row > 1 then previous = c2d(substr(value, 66 + 4 * (row - 2), 4))
    boundary = c2d(substr(value, 66 + 4 * (row - 1), 4))
    if boundary < previous | boundary > nf then return -1
    return boundary - previous
  end
  when operation = 'ROW_COUNT' then do
    if arg() \= 2 then return -1
    value = arg(2)
    if table_valid() then return c2d(substr(value, 58, 4))
    if value_valid() then do
      if substr(value, 8, 2) == 'D0' then return 0
    end
    return -1
  end
  when operation = 'OK' then do
    if arg() \= 2 then return 0
    value = arg(2)
    if value_valid() = 0 then return 0
    if substr(value, 9, 1) \== '1' then return 0
    if pos(substr(value, 8, 1), 'TDR') > 0 then return table_valid()
    return 1
  end
  when operation = 'TEXT' then do
    if arg() \= 2 then return ''
    value = arg(2)
    if value_valid() = 0 then return ''
    if substr(value, 9, 1) \== '1' then return ''
    if pos(substr(value, 8, 1), 'FW') = 0 then return ''
    return substr(value, 58)
  end
  when wordpos(operation, 'ERROR_CODE ERROR_RECORD ERROR_FIELD ERROR_LIMIT ERROR') > 0 then do
    value = arg(2)
    if arg() \= 2 | value_valid() = 0 then value = misuse()
    select
      when operation = 'ERROR_CODE' then return strip(substr(value, 10, 20), 'T')
      when operation = 'ERROR_RECORD' then return c2d(substr(value, 30, 4))
      when operation = 'ERROR_FIELD' then return c2d(substr(value, 34, 4))
      when operation = 'ERROR_LIMIT' then return strip(substr(value, 38, 20), 'T')
      otherwise return message(strip(substr(value, 10, 20), 'T'), strip(substr(value, 38, 20), 'T'))
    end
  end
  otherwise return misuse()
end

/* @return private checked result header; all positions are byte offsets. */
header: procedure
  parse arg kind, ok, code, record, column, limit
  return 'RXCSV01' || kind || ok || left(code, 20) || d2c(record, 4) || d2c(column, 4) || left(limit, 20)

misuse: procedure
  return header('E', 0, 'E_ARGUMENT', 0, 0, '')

whole: procedure
  parse arg value
  return datatype(value, 'W')

/* These helpers expose only the library activation's private value. Passing
 * the whole table through per-cell PARSE ARG would copy it on Regina.
 * Validate the envelope in constant time. Private table entries are range
 * checked as they are read; the writer also walks every row and field.
 */
value_valid: procedure expose value
  if length(value) < 57 then return 0
  if left(value, 7) \== 'RXCSV01' then return 0
  kind = substr(value, 8, 1)
  if pos(kind, 'TDRWFE') = 0 then return 0
  ok = substr(value, 9, 1)
  if ok == '0' then do
    if length(value) \= 57 then return 0
    code = strip(substr(value, 10, 20), 'T')
    return wordpos(code, 'E_QUOTE E_AFTER_QUOTE E_UNCLOSED_QUOTE E_RECORD_SEPARATOR E_NUL E_EMPTY_ROW E_LIMIT E_ARGUMENT') > 0
  end
  if ok \== '1' | kind == 'E' then return 0
  if substr(value, 10, 20) \== copies(' ', 20) then return 0
  if substr(value, 30, 8) \== copies('00'x, 8) then return 0
  if substr(value, 38, 20) \== copies(' ', 20) then return 0
  return 1

table_valid: procedure expose value
  if value_valid() = 0 then return 0
  if substr(value, 9, 1) \== '1' then return 0
  if pos(substr(value, 8, 1), 'TDR') = 0 | length(value) < 65 then return 0
  nr = c2d(substr(value, 58, 4))
  nf = c2d(substr(value, 62, 4))
  if 65 + 4 * nr + 8 * nf > length(value) then return 0
  if nr = 0 then return nf = 0 & length(value) = 65
  if substr(value, 8, 1) == 'R' & nr \= 1 then return 0
  return c2d(substr(value, 66 + 4 * (nr - 1), 4)) = nf

cell_valid: procedure expose value
  parse arg flat
  nr = c2d(substr(value, 58, 4))
  nf = c2d(substr(value, 62, 4))
  if flat < 1 | flat > nf then return 0
  entry = 66 + 4 * nr + 8 * (flat - 1)
  start = c2d(substr(value, entry, 4))
  size = c2d(substr(value, entry + 4, 4))
  return start + size <= length(value) - 65 - 4 * nr - 8 * nf

cell_text: procedure expose value
  parse arg flat
  nr = c2d(substr(value, 58, 4))
  nf = c2d(substr(value, 62, 4))
  entry = 66 + 4 * nr + 8 * (flat - 1)
  start = c2d(substr(value, entry, 4))
  size = c2d(substr(value, entry + 4, 4))
  return substr(value, 66 + 4 * nr + 8 * nf + start, size)

locate: procedure expose value
  parse arg row, column
  if table_valid() = 0 | whole(row) = 0 | whole(column) = 0 then return 0
  nr = c2d(substr(value, 58, 4))
  nf = c2d(substr(value, 62, 4))
  if row < 1 | row > nr | column < 1 then return 0
  previous = 0
  if row > 1 then previous = c2d(substr(value, 66 + 4 * (row - 2), 4))
  boundary = c2d(substr(value, 66 + 4 * (row - 1), 4))
  if previous > boundary | boundary > nf | column > boundary - previous then return 0
  flat = previous + column
  if cell_valid(flat) = 0 then return 0
  return flat

/* All assembly is over private local stems. Consumed chunks are released
 * each round; no repeated left-fold copying of an ever-growing table.
 */
balanced: procedure expose parts.
  parse arg count
  if count = 0 then return ''
  do while count > 1
    output = 0
    do input = 1 to count by 2
      output = output + 1
      next = input + 1
      if next <= count then parts.output = parts.input || parts.next
      else parts.output = parts.input
      if input > output then drop parts.input
      if next <= count then drop parts.next
    end
    count = output
  end
  return parts.1

assemble: procedure expose cells. ends.
  parse arg kind, records, fields
  do i = 1 to records
    parts.i = d2c(ends.i, 4)
  end
  row_index = balanced(records)
  offset = 0
  do i = 1 to fields
    size = length(cells.i)
    if offset + size > 4294967295 then return misuse()
    parts.i = d2c(offset, 4) || d2c(size, 4)
    offset = offset + size
  end
  cell_index = balanced(fields)
  do i = 1 to fields
    parts.i = cells.i
  end
  payload = balanced(fields)
  return header(kind, 1, '', 0, 0, '') || d2c(records, 4) || d2c(fields, 4) || row_index || cell_index || payload

/* UTF-8 is an explicit binding boundary. ASCII runs are skipped in native
 * VERIFY; non-ASCII scalars reject truncation, overlongs and surrogates.
 */
utf8_valid: procedure
  parse arg text
  ascii = xrange('00'x, '7f'x)
  if verify(text, ascii, 'N') = 0 then return 1
  total = length(text)
  start = 1
  do while start <= total
    size = min(4096, total - start + 1)
    /* Keep a scalar crossing the chunk boundary together. An invalid run
     * cannot force unbounded lookahead; the unchanged scalar check rejects it.
     */
    do extra = 1 to 3
      if start + size > total then leave
      byte = c2d(substr(text, start + size, 1))
      if byte < 128 | byte > 191 then leave
      size = size + 1
    end
    if utf8_chunk(substr(text, start, size)) = 0 then return 0
    start = start + size
  end
  return 1

/* Bound BIF value arguments: Regina copies large operands in this loop. */
utf8_chunk: procedure
  parse arg text
  ascii = xrange('00'x, '7f'x)
  p = 1
  size = length(text)
  do while p <= size
    p = verify(text, ascii, 'N', p)
    if p = 0 then return 1
    lead = c2d(substr(text, p, 1))
    select
      when lead >= 194 & lead <= 223 then count = 2
      when lead >= 224 & lead <= 239 then count = 3
      when lead >= 240 & lead <= 244 then count = 4
      otherwise return 0
    end
    if p + count - 1 > size then return 0
    second = c2d(substr(text, p + 1, 1))
    if second < 128 | second > 191 then return 0
    if lead = 224 & second < 160 then return 0
    if lead = 237 & second > 159 then return 0
    if lead = 240 & second < 144 then return 0
    if lead = 244 & second > 143 then return 0
    do i = 2 to count - 1
      byte = c2d(substr(text, p + i, 1))
      if byte < 128 | byte > 191 then return 0
    end
    p = p + count
  end
  return 1

/* @param text UTF-8 bytes @return owned checked parse document. */
parse_csv: procedure
  parse arg text
  size = length(text)
  if size > 16777216 then return header('D', 0, 'E_LIMIT', 0, 0, 'input_bytes')
  if utf8_valid(text) = 0 then return misuse()
  p = 1
  if left(text, 3) == 'efbbbf'x then p = 4
  if p > size then return assemble('D', 0, 0)
  record = 1
  column = 1
  total = 1
  state = 0
  start = p
  decoded = 0
  code = ''
  limit = ''
  frame = ',"' || '0d0a00'x
  quoted_frame = '"' || '00'x
  do forever
    if state = 0 then do
      start = p
      if p <= size then do
        if substr(text, p, 1) == '"' then do
          state = 2
          p = p + 1
          start = p
        end
      end
    end
    if state = 2 then do
      q = verify(text, quoted_frame, 'M', p)
      if q = 0 then q = size + 1
      decoded = decoded + q - p
      if decoded > 1048576 then do
        code = 'E_LIMIT'; limit = 'field_bytes'; leave
      end
      if q > size then do
        code = 'E_UNCLOSED_QUOTE'; leave
      end
      if substr(text, q, 1) == '00'x then do
        code = 'E_NUL'; leave
      end
      after = verify(text, '"', 'N', q)
      if after = 0 then after = size + 1
      quotes = after - q
      decoded = decoded + quotes % 2
      if decoded > 1048576 then do
        code = 'E_LIMIT'; limit = 'field_bytes'; leave
      end
      p = after
      if quotes // 2 = 1 then do
        value = changestr('""', substr(text, start, after - start - 1), '"')
        state = 3
      end
      iterate
    end
    if state \= 3 then do
      q = verify(text, frame, 'M', p)
      if q = 0 then q = size + 1
      decoded = decoded + q - p
      if decoded > 1048576 then do
        code = 'E_LIMIT'; limit = 'field_bytes'; leave
      end
      if q > p then state = 1
      p = q
    end
    byte = ''
    if p <= size then byte = substr(text, p, 1)
    if byte == '00'x then do
      code = 'E_NUL'; leave
    end
    if byte == '' | byte == ',' | byte == '0a'x | byte == '0d'x then do
      step = 1
      if byte == '0d'x then do
        if substr(text, p + 1, 1) \== '0a'x then do
          code = 'E_RECORD_SEPARATOR'; leave
        end
        step = 2
      end
      if state \= 3 then value = substr(text, start, p - start)
      cells.total = value
      decoded = 0
      state = 0
      p = p + step
      if byte == ',' then do
        column = column + 1
        total = total + 1
        if column > 1024 then do
          code = 'E_LIMIT'; limit = 'fields_per_record'; leave
        end
      end
      else do
        ends.record = total
        if byte == '' | p > size then return assemble('D', record, total)
        record = record + 1
        column = 1
        total = total + 1
        if record > 10000 then do
          code = 'E_LIMIT'; limit = 'records'; leave
        end
      end
      if total > 100000 then do
        code = 'E_LIMIT'; limit = 'total_fields'; leave
      end
      iterate
    end
    if state = 3 then code = 'E_AFTER_QUOTE'
    else code = 'E_QUOTE'
    leave
  end
  return header('D', 0, code, record, column, limit)

/* @param table owned row/table/document @return checked canonical output. */
write_csv: procedure expose value
  if table_valid() = 0 then return misuse()
  records = c2d(substr(value, 58, 4))
  fields = c2d(substr(value, 62, 4))
  previous = 0
  total = 0
  written = 0
  code = ''
  limit = ''
  do record = 1 to records
    column = 1
    if record > 10000 then do
      code = 'E_LIMIT'; limit = 'records'; leave
    end
    boundary = c2d(substr(value, 66 + 4 * (record - 1), 4))
    if boundary < previous | boundary > fields then return misuse()
    count = boundary - previous
    if count = 0 then return header('W', 0, 'E_EMPTY_ROW', record, 0, '')
    do column = 1 to count
      if column > 1024 then do
        code = 'E_LIMIT'; limit = 'fields_per_record'; leave
      end
      total = total + 1
      if total > 100000 then do
        code = 'E_LIMIT'; limit = 'total_fields'; leave
      end
      if cell_valid(total) = 0 then return misuse()
      text = cell_text(total)
      size = length(text)
      if utf8_valid(text) = 0 then return misuse()
      nul = pos('00'x, text)
      if nul > 0 & nul <= 1048577 then do
        code = 'E_NUL'; leave
      end
      if size > 1048576 then do
        code = 'E_LIMIT'; limit = 'field_bytes'; leave
      end
      quotes = countstr('"', text)
      quoted = size = 0 | quotes > 0 | verify(text, ',' || '0d0a'x, 'M') > 0
      if record = 1 & column = 1 & left(text, 3) == 'efbbbf'x then quoted = 1
      contribution = size + quotes + 2 * quoted
      if column > 1 then contribution = contribution + 1
      if column = count then contribution = contribution + 2
      if written + contribution > 16777216 then do
        code = 'E_LIMIT'; limit = 'output_bytes'; leave
      end
      written = written + contribution
      if quoted then text = '"' || changestr('"', text, '""') || '"'
      if column > 1 then text = ',' || text
      if column = count then text = text || '0d0a'x
      parts.total = text
    end
    if code \== '' then leave
    previous = boundary
  end
  if code \== '' then return header('W', 0, code, record, column, limit)
  return header('W', 1, '', 0, 0, '') || balanced(total)

message: procedure
  parse arg code, limit
  select
    when code == '' then return ''
    when code == 'E_ARGUMENT' then return 'Invalid CSV operation, value, coordinate or UTF-8 text'
    when code == 'E_QUOTE' then return 'Quote in an unquoted field'
    when code == 'E_AFTER_QUOTE' then return 'Unexpected character after closing quote'
    when code == 'E_UNCLOSED_QUOTE' then return 'Unclosed quoted field'
    when code == 'E_RECORD_SEPARATOR' then return 'Lone CR outside quotes'
    when code == 'E_NUL' then return 'NUL is not permitted in CSV text'
    when code == 'E_EMPTY_ROW' then return 'A CSV row must contain at least one field'
    otherwise return 'CSV resource limit exceeded: ' || limit
  end
