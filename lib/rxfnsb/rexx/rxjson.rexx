options levelb
namespace rxjson expose jsonvalid jsontype jsonget jsoncount jsonmembers jsonquote jsonunquote jsonarray jsonobject
import rxfnsb

jsonvalid: procedure = .int
  arg json = .string
  if word(_json_root_info(json), 1) = "error" then return 0
  return 1

jsontype: procedure = .string
  arg json = .string, path = .string
  return word(_json_locate(json, path), 1)

jsonget: procedure = .string
  arg json = .string, path = .string
  info = _json_locate(json, path)
  kind = word(info, 1)
  start = word(info, 2)
  after = word(info, 3)

  if kind = "missing" | kind = "error" then return ""
  if kind = "string" then return _json_unquote_span(json, start, after)
  return substr(json, start, after - start)

jsoncount: procedure = .int
  arg json = .string, path = .string
  info = _json_locate(json, path)
  kind = word(info, 1)
  if kind = "missing" | kind = "error" then return -1
  if kind = "array" then return _json_array_count(json, word(info, 2), word(info, 3))
  if kind = "object" then return _json_object_count(json, word(info, 2), word(info, 3))
  return 0

jsonmembers: procedure = .int
  arg json = .string, path = .string, expose names = .string[]
  info = _json_locate(json, path)
  kind = word(info, 1)
  if kind = "missing" | kind = "error" then return -1
  if kind = "array" then return _json_array_members(json, word(info, 2), word(info, 3), names)
  if kind = "object" then return _json_object_members(json, word(info, 2), word(info, 3), names)
  return 0

jsonquote: procedure = .string
  arg text = .string
  out = '"'
  do i = 1 to length(text)
    c = substr(text, i, 1)
    code = c2d(c)
    if c = '"' then out = out || '\"'
    else if c = '\' then out = out || '\\'
    else if c = '08'x then out = out || '\b'
    else if c = '0c'x then out = out || '\f'
    else if c = '0a'x then out = out || '\n'
    else if c = '0d'x then out = out || '\r'
    else if c = '09'x then out = out || '\t'
    else if code < 32 then out = out || '\u' || right(d2x(code), 4, '0')
    else out = out || c
  end
  return out || '"'

jsonunquote: procedure = .string
  arg json = .string
  info = _json_root_info(json)
  if word(info, 1) \= "string" then return ""
  return _json_unquote_span(json, word(info, 2), word(info, 3))

jsonarray: procedure = .string
  arg values = .string[]
  out = "["
  do i = 1 to values.0
    if jsonvalid(values[i]) = 0 then return ""
    if i > 1 then out = out || ","
    out = out || values[i]
  end
  return out || "]"

jsonobject: procedure = .string
  arg keys = .string[], values = .string[]
  if keys.0 \= values.0 then return ""
  out = "{"
  do i = 1 to keys.0
    if jsonvalid(values[i]) = 0 then return ""
    if i > 1 then out = out || ","
    out = out || jsonquote(keys[i]) || ":" || values[i]
  end
  return out || "}"

_json_locate: procedure = .string
  arg json = .string, path = .string
  info = _json_root_info(json)
  if word(info, 1) = "error" then return info
  if path = "" then return info

  p = 1
  do while p <= length(path)
    if substr(path, p, 1) = "." then do
      p = p + 1
      iterate
    end

    if substr(path, p, 1) = "[" then do
      close = pos("]", path, p + 1)
      if close = 0 then return "error 0 0"
      index_text = substr(path, p + 1, close - p - 1)
      if _json_is_digits(index_text) = 0 then return "error 0 0"
      info = _json_child_index(json, info, index_text)
      if word(info, 1) = "missing" | word(info, 1) = "error" then return info
      p = close + 1
      iterate
    end

    start = p
    do while p <= length(path)
      c = substr(path, p, 1)
      if c = "." | c = "[" then leave
      p = p + 1
    end
    segment = substr(path, start, p - start)
    if segment = "" then return "error 0 0"
    if word(info, 1) = "array" & _json_is_digits(segment) = 1 then info = _json_child_index(json, info, segment)
    else info = _json_child_key(json, info, segment)
    if word(info, 1) = "missing" | word(info, 1) = "error" then return info
  end

  return info

_json_root_info: procedure = .string
  arg json = .string
  info = _json_value_info(json, 1)
  if word(info, 1) = "error" then return info
  after = _json_skip_ws(json, word(info, 3))
  if after \= length(json) + 1 then return "error 0 0"
  return info

_json_value_info: procedure = .string
  arg json = .string, posn = .int
  posn = _json_skip_ws(json, posn)
  if posn > length(json) then return "error 0 0"
  c = substr(json, posn, 1)
  if c = '"' then return _json_string_info(json, posn)
  if c = "{" then return _json_object_info(json, posn)
  if c = "[" then return _json_array_info(json, posn)
  if c = "t" then do
    if substr(json, posn, 4) = "true" then return "boolean " || posn || " " || (posn + 4)
    return "error 0 0"
  end
  if c = "f" then do
    if substr(json, posn, 5) = "false" then return "boolean " || posn || " " || (posn + 5)
    return "error 0 0"
  end
  if c = "n" then do
    if substr(json, posn, 4) = "null" then return "null " || posn || " " || (posn + 4)
    return "error 0 0"
  end
  if c = "-" | _json_is_digit(c) = 1 then return _json_number_info(json, posn)
  return "error 0 0"

_json_string_info: procedure = .string
  arg json = .string, posn = .int
  if substr(json, posn, 1) \= '"' then return "error 0 0"
  p = posn + 1
  do while p <= length(json)
    c = substr(json, p, 1)
    if c = '"' then return "string " || posn || " " || (p + 1)
    if c2d(c) < 32 then return "error 0 0"
    if c = '\' then do
      p = p + 1
      if p > length(json) then return "error 0 0"
      esc = substr(json, p, 1)
      if pos(esc, '"\/bfnrt') > 0 then do
        p = p + 1
        iterate
      end
      if esc = "u" then do
        if p + 4 > length(json) then return "error 0 0"
        hex = substr(json, p + 1, 4)
        if _json_is_hex4(hex) = 0 then return "error 0 0"
        p = p + 5
        iterate
      end
      return "error 0 0"
    end
    p = p + 1
  end
  return "error 0 0"

_json_number_info: procedure = .string
  arg json = .string, posn = .int
  p = posn
  if substr(json, p, 1) = "-" then p = p + 1
  if p > length(json) then return "error 0 0"
  c = substr(json, p, 1)
  if c = "0" then p = p + 1
  else if _json_is_digit19(c) = 1 then do
    do while p <= length(json) & _json_is_digit(substr(json, p, 1)) = 1
      p = p + 1
    end
  end
  else return "error 0 0"

  if p <= length(json) & substr(json, p, 1) = "." then do
    p = p + 1
    if p > length(json) | _json_is_digit(substr(json, p, 1)) = 0 then return "error 0 0"
    do while p <= length(json) & _json_is_digit(substr(json, p, 1)) = 1
      p = p + 1
    end
  end

  if p <= length(json) & pos(substr(json, p, 1), "eE") > 0 then do
    p = p + 1
    if p <= length(json) & pos(substr(json, p, 1), "+-") > 0 then p = p + 1
    if p > length(json) | _json_is_digit(substr(json, p, 1)) = 0 then return "error 0 0"
    do while p <= length(json) & _json_is_digit(substr(json, p, 1)) = 1
      p = p + 1
    end
  end

  return "number " || posn || " " || p

_json_object_info: procedure = .string
  arg json = .string, posn = .int
  p = posn + 1
  p = _json_skip_ws(json, p)
  if p <= length(json) & substr(json, p, 1) = "}" then return "object " || posn || " " || (p + 1)
  do while p <= length(json)
    key_info = _json_string_info(json, p)
    if word(key_info, 1) = "error" then return key_info
    p = _json_skip_ws(json, word(key_info, 3))
    if p > length(json) | substr(json, p, 1) \= ":" then return "error 0 0"
    value_info = _json_value_info(json, p + 1)
    if word(value_info, 1) = "error" then return value_info
    p = _json_skip_ws(json, word(value_info, 3))
    if p <= length(json) & substr(json, p, 1) = "," then do
      p = _json_skip_ws(json, p + 1)
      iterate
    end
    if p <= length(json) & substr(json, p, 1) = "}" then return "object " || posn || " " || (p + 1)
    return "error 0 0"
  end
  return "error 0 0"

_json_array_info: procedure = .string
  arg json = .string, posn = .int
  p = posn + 1
  p = _json_skip_ws(json, p)
  if p <= length(json) & substr(json, p, 1) = "]" then return "array " || posn || " " || (p + 1)
  do while p <= length(json)
    value_info = _json_value_info(json, p)
    if word(value_info, 1) = "error" then return value_info
    p = _json_skip_ws(json, word(value_info, 3))
    if p <= length(json) & substr(json, p, 1) = "," then do
      p = _json_skip_ws(json, p + 1)
      iterate
    end
    if p <= length(json) & substr(json, p, 1) = "]" then return "array " || posn || " " || (p + 1)
    return "error 0 0"
  end
  return "error 0 0"

_json_child_key: procedure = .string
  arg json = .string, info = .string, wanted = .string
  if word(info, 1) \= "object" then return "missing 0 0"
  p = _json_skip_ws(json, word(info, 2) + 1)
  endpos = word(info, 3) + 0
  if p < endpos & substr(json, p, 1) = "}" then return "missing 0 0"
  do while p < endpos
    key_info = _json_string_info(json, p)
    if word(key_info, 1) = "error" then return key_info
    candidate = _json_unquote_span(json, word(key_info, 2), word(key_info, 3))
    p = _json_skip_ws(json, word(key_info, 3))
    if p >= endpos | substr(json, p, 1) \= ":" then return "error 0 0"
    value_info = _json_value_info(json, p + 1)
    if word(value_info, 1) = "error" then return value_info
    if candidate = wanted then return value_info
    p = _json_skip_ws(json, word(value_info, 3))
    if p < endpos & substr(json, p, 1) = "," then p = _json_skip_ws(json, p + 1)
    else leave
  end
  return "missing 0 0"

_json_child_index: procedure = .string
  arg json = .string, info = .string, index = .int
  if word(info, 1) \= "array" then return "missing 0 0"
  if index < 1 then return "missing 0 0"
  p = _json_skip_ws(json, word(info, 2) + 1)
  endpos = word(info, 3) + 0
  current = 1
  if p < endpos & substr(json, p, 1) = "]" then return "missing 0 0"
  do while p < endpos
    value_info = _json_value_info(json, p)
    if word(value_info, 1) = "error" then return value_info
    if current = index then return value_info
    current = current + 1
    p = _json_skip_ws(json, word(value_info, 3))
    if p < endpos & substr(json, p, 1) = "," then p = _json_skip_ws(json, p + 1)
    else leave
  end
  return "missing 0 0"

_json_object_count: procedure = .int
  arg json = .string, start = .int, after = .int
  p = _json_skip_ws(json, start + 1)
  count = 0
  if p < after & substr(json, p, 1) = "}" then return 0
  do while p < after
    key_info = _json_string_info(json, p)
    if word(key_info, 1) = "error" then return -1
    p = _json_skip_ws(json, word(key_info, 3))
    if p >= after | substr(json, p, 1) \= ":" then return -1
    value_info = _json_value_info(json, p + 1)
    if word(value_info, 1) = "error" then return -1
    count = count + 1
    p = _json_skip_ws(json, word(value_info, 3))
    if p < after & substr(json, p, 1) = "," then p = _json_skip_ws(json, p + 1)
    else leave
  end
  return count

_json_array_count: procedure = .int
  arg json = .string, start = .int, after = .int
  p = _json_skip_ws(json, start + 1)
  count = 0
  if p < after & substr(json, p, 1) = "]" then return 0
  do while p < after
    value_info = _json_value_info(json, p)
    if word(value_info, 1) = "error" then return -1
    count = count + 1
    p = _json_skip_ws(json, word(value_info, 3))
    if p < after & substr(json, p, 1) = "," then p = _json_skip_ws(json, p + 1)
    else leave
  end
  return count

_json_object_members: procedure = .int
  arg json = .string, start = .int, after = .int, expose names = .string[]
  p = _json_skip_ws(json, start + 1)
  count = 0
  if p < after & substr(json, p, 1) = "}" then return 0
  do while p < after
    key_info = _json_string_info(json, p)
    if word(key_info, 1) = "error" then return -1
    p = _json_skip_ws(json, word(key_info, 3))
    if p >= after | substr(json, p, 1) \= ":" then return -1
    value_info = _json_value_info(json, p + 1)
    if word(value_info, 1) = "error" then return -1
    count = count + 1
    names[count] = _json_unquote_span(json, word(key_info, 2), word(key_info, 3))
    p = _json_skip_ws(json, word(value_info, 3))
    if p < after & substr(json, p, 1) = "," then p = _json_skip_ws(json, p + 1)
    else leave
  end
  return count

_json_array_members: procedure = .int
  arg json = .string, start = .int, after = .int, expose names = .string[]
  count = _json_array_count(json, start, after)
  if count < 0 then return -1
  do i = 1 to count
    names[i] = i
  end
  return count

_json_unquote_span: procedure = .string
  arg json = .string, start = .int, after = .int
  out = ""
  p = start + 1
  do while p < after - 1
    c = substr(json, p, 1)
    if c = '\' then do
      p = p + 1
      if p >= after then return ""
      esc = substr(json, p, 1)
      if esc = '"' then out = out || '"'
      else if esc = '\' then out = out || '\'
      else if esc = "/" then out = out || "/"
      else if esc = "b" then out = out || '08'x
      else if esc = "f" then out = out || '0c'x
      else if esc = "n" then out = out || '0a'x
      else if esc = "r" then out = out || '0d'x
      else if esc = "t" then out = out || '09'x
      else if esc = "u" then do
        hex = substr(json, p + 1, 4)
        if _json_is_hex4(hex) = 0 then return ""
        code = x2d(hex)
        if code < 128 then out = out || x2c(right(hex, 2, "0"))
        else out = out || "?"
        p = p + 4
      end
      else return ""
    end
    else out = out || c
    p = p + 1
  end
  return out

_json_skip_ws: procedure = .int
  arg json = .string, posn = .int
  do while posn <= length(json)
    c = substr(json, posn, 1)
    if c = " " | c = '09'x | c = '0a'x | c = '0d'x then posn = posn + 1
    else leave
  end
  return posn

_json_is_digits: procedure = .int
  arg text = .string
  if text = "" then return 0
  do i = 1 to length(text)
    if _json_is_digit(substr(text, i, 1)) = 0 then return 0
  end
  return 1

_json_is_digit: procedure = .int
  arg c = .string
  if c >= "0" & c <= "9" then return 1
  return 0

_json_is_digit19: procedure = .int
  arg c = .string
  if c >= "1" & c <= "9" then return 1
  return 0

_json_is_hex4: procedure = .int
  arg text = .string
  if length(text) \= 4 then return 0
  do i = 1 to 4
    c = substr(text, i, 1)
    if _json_is_digit(c) = 0 & (c < "a" | c > "f") & (c < "A" | c > "F") then return 0
  end
  return 1
