  byte: method = .int
    arg at = .int
    if at < 0 | at >= <blen>(_bytes) then return -1
    return <at..u8>(at) _bytes
  symbolstart: method = .int
    arg c = .int
    return c = 95 | (c >= 65 & c <= 90) | (c >= 97 & c <= 122)
  symbolpart: method = .int
    arg c = .int
    return symbolstart(c) | (c >= 48 & c <= 57) | c = 46
  scan: method = .void
    at = 0; line = 1; value = ''; start = 0; firstline = 1; extent = <blen>(_bytes)
    do while at < extent & _status = 0
      c = byte(at); start = at; firstline = line
      select
        when c = 32 | c = 9 | c = 10 | c = 13 then do
          if c = 13 | (c = 10 & byte(at - 1) <> 13) then line = line + 1
          at = at + 1
        end
        when c = 47 & byte(at + 1) = 42 then do
          nesting = 1; at = at + 2
          do while nesting > 0 & at < extent
            if byte(at) = 47 & byte(at + 1) = 42 then do
              nesting = nesting + 1; at = at + 2
            end
            else if byte(at) = 42 & byte(at + 1) = 47 then do
              nesting = nesting - 1; at = at + 2
            end
            else do
              if byte(at) = 13 | (byte(at) = 10 & byte(at - 1) <> 13) then line = line + 1
              at = at + 1
            end
          end
          if nesting > 0 then do
            call emit('error','',firstline,start); call fail('unterminated Rexx comment',_count)
          end
        end
        when c = 39 | c = 34 then do
          quote = c; at = at + 1; value = ''; closed = 0
          do while at < extent
            c = byte(at)
            if c = quote then do
              if byte(at + 1) = quote then do
                at = at + 2
              end
              else do
                at = at + 1; closed = 1; leave
              end
            end
            else do
              if c = 13 | (c = 10 & byte(at - 1) <> 13) then line = line + 1
              at = at + 1
            end
          end
          if closed then value = changestr(d2c(quote) || d2c(quote),binsubstr(_bytes,start + 2,at - start - 2) as .string,d2c(quote))
          call emit('string',value,firstline,start)
          if closed = 0 then call fail('unterminated literal',_count)
        end
        when (c = 85 | c = 117) & byte(at + 1) = 43 then do
          at = at + 2; value = ''
          do while pos(translate(d2c(max(0,byte(at)))),'0123456789ABCDEF') > 0 & byte(at) >= 0
            value = value || d2c(byte(at)); at = at + 1
          end
          call emit('codepoint',value,line,start)
          if value = '' | length(value) > 6 then call fail('expected codepoint U+0000..U+10FFFF',_count)
        end
        when symbolstart(c) then do
          at = at + 1
          do while symbolpart(byte(at))
            at = at + 1
          end
          value = binsubstr(_bytes,start + 1,at - start) as .string
          call emit('symbol',value,line,start)
          if right(value,1) = '.' | pos('..',value) > 0 then call fail('invalid static compound name',_count)
        end
        when c >= 48 & c <= 57 then do
          at = at + 1
          do while byte(at) >= 48 & byte(at) <= 57
            at = at + 1
          end
          call emit('integer',binsubstr(_bytes,start + 1,at - start) as .string,line,start)
        end
        otherwise do
          if c < 128 & pos(d2c(c),':=|()?*+!&{},') > 0 then call emit(d2c(c),d2c(c),line,start)
          else do
            call emit('error','',line,start); call fail('unsupported authored character',_count)
          end
          at = at + 1
        end
      end
    end
    if _status = 0 then call emit('eof','',line,at)
    return
  setunion: method = .llcharset..ranges
    result = setdifference()
    do while _at <= _end & _kind[_at] = '|' & _status = 0
      _at = _at + 1; other = setdifference(); call result.unite(other)
    end
    return result
  setdifference: method = .llcharset..ranges
    result = setatom()
    do while _at <= _end & wordat(_at) = 'except' & _status = 0
      _at = _at + 1; other = setatom(); call result.subtract(other)
    end
    return result
  setatom: method = .llcharset..ranges
    result = .llcharset..ranges(); _depth = _depth + 1
    if _depth > 128 then do
      _status = 4; _error = 'LL_IMPLEMENTATION_LIMIT: seed reader depth exceeds 128; this stack ceiling requires an iterative implementation to grow'; return result
    end
    select
      when _kind[_at] = '(' then do
        _at = _at + 1; result = setunion(); ok = expect(')')
      end
      when wordat(_at) = 'scalar' then do
        if _encoding <> 'utf8' then call fail('scalar requires lexer input utf8',_at)
        _at = _at + 1; call result.add(0,55295); call result.add(57344,1114111)
      end
      when _kind[_at] = 'symbol' then do
        if left(lower(_text[_at]),8) = 'unicode.' & index(_text[_at]) = 0 then do
          result = property(_text[_at],_at); _at = _at + 1
        end
        else do
        named = index(_text[_at]); _at = _at + 1
        if named = 0 then call fail('unknown charset',_at - 1)
        else if _type[named] <> 'charset' | _lexical[named] = 0 then call fail('expected lexical charset',_at - 1)
        else result = charset(named)
        end
      end
      otherwise do
        low = codepoint(); high = low
        if wordat(_at) = 'to' then do
          _at = _at + 1; high = codepoint()
        end
        if low > high then call fail('inverted range',_at - 1)
        call result.add(low,high)
      end
    end
    _depth = _depth - 1
    return result
  alternative: method = .int
    result = sequence()
    do while _at <= _end & _kind[_at] = '|' & _status = 0
      at = _at; _at = _at + 1; child = sequence(); op = 3
      if _profile = 'contextfree' then op = 4
      result = addnode(op,result,child,'choice',_line[at])
    end
    return result
  sequence: method = .int
    result = 0
    do while _at <= _end & _kind[_at] <> '|' & _kind[_at] <> ')' & _status = 0
      at = _at; child = term()
      if result = 0 then result = child
      else result = addnode(2,result,child,'sequence',_line[at])
    end
    if result = 0 then call fail('empty alternative: write a quoted empty literal explicitly',_at)
    return result
  term: method = .int
    _depth = _depth + 1
    if _depth > 128 then do
      _status = 4; _error = 'LL_IMPLEMENTATION_LIMIT: seed reader depth exceeds 128; this stack ceiling requires an iterative implementation to grow'; return 0
    end
    at = _at; kind = _kind[at]; _at = _at + 1; result = 0
    select kind
      when '(' then do
        result = alternative(); ok = expect(')')
      end
      when '!' then do
        child = term(); result = addnode(8,child,0,'negative lookahead',_line[at])
      end
      when '&' then do
        child = term(); result = addnode(9,child,0,'positive lookahead',_line[at])
      end
      when 'string' then result = literal(_text[at] as .binary,_line[at])
      when 'codepoint' then do
        if _lexmode = 0 then call fail('codepoint atoms require a lexical rule',at)
        _at = at; scalar = codepoint(); members = .llcharset..ranges()
        call members.add(scalar,scalar)
        result = llcharset..lower(_graph,members,_encoding = 'utf8',_line[at])
      end
      when 'symbol' then do
        named = index(_text[at])
        if named = 0 then call fail('undefined symbol ' || _text[at],at)
        else if _handle[named] = 0 then call fail('symbol is not a rule, token or charset',at)
        else result = _handle[named]
      end
      otherwise call fail('expected rule, quoted literal or grouped expression',at)
    end
    if _at <= _end & _status = 0 then do
      kind = _kind[_at]; op = 0
      if kind = '{' then do
        result = repetition(result)
      end
      if kind = '*' then op = 7
      if kind = '+' then op = 14
      if kind = '?' then op = 15
      if op > 0 then do
        if op = 7 then result = addnode(7,result,0,'repeat *',_line[_at])
        else if op = 14 then do
          tail = addnode(7,result,0,'repeat *',_line[_at])
          result = addnode(2,result,tail,'repeat +',_line[_at])
        end
        else do
          empty = addnode(6,0,0,'empty',_line[_at]); choiceop = 3
          if _profile = 'contextfree' then choiceop = 4
          result = addnode(choiceop,result,empty,'optional',_line[_at])
        end
        _at = _at + 1
      end
    end
    if _at <= _end & wordat(_at) = 'as' & _status = 0 then do
      _at = _at + 1; result = capture(result,_at); _at = _at + 1
    end
    _depth = _depth - 1
    return result
  repetition: method = .int
    arg child = .int
    at = _at; _at = _at + 1; low = number(); high = low
    if _kind[_at] = ',' then do
      _at = _at + 1; high = -1
      if _kind[_at] <> '}' then high = number()
    end
    ok = expect('}')
    return repeatnode(child,low,high,_line[at])
