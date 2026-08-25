/*
 * Retained re2c input for the first Level L emitter experiment.
 *
 * The authored Level L syntax is documented separately. This file uses re2c
 * only to test whether its automaton can be emitted as valid cREXX Level L.
 */

options levell

namespace level_l_re2c expose level_l_re2c_lex level_l_re2c_status level_l_re2c_error_offset level_l_re2c_token_count level_l_re2c_token_kind level_l_re2c_token_start level_l_re2c_token_length level_l_re2c_token_value level_l_re2c_token_name
import rxfnsb

level_l_re2c_constants: procedure expose TOK_EOF TOK_NUMBER TOK_IDENT TOK_PLUS TOK_MINUS TOK_STAR TOK_SLASH TOK_LPAREN TOK_RPAREN TOKEN_KIND TOKEN_START TOKEN_LENGTH TOKEN_VALUE TOKEN_SIZE
  constant TOK_EOF = 0
  constant TOK_NUMBER = 1
  constant TOK_IDENT = 2
  constant TOK_PLUS = 3
  constant TOK_MINUS = 4
  constant TOK_STAR = 5
  constant TOK_SLASH = 6
  constant TOK_LPAREN = 7
  constant TOK_RPAREN = 8

  constant TOKEN_KIND = 0
  constant TOKEN_START = 2
  constant TOKEN_LENGTH = 6
  constant TOKEN_VALUE = 8
  constant TOKEN_SIZE = 16
  return

level_l_re2c_lex: procedure = .binary expose _level_l_status _level_l_error_offset
  arg source = .string

  _level_l_status = 0
  _level_l_error_offset = 0
  tokens = .binary
  count = 0
  pos = 0
  start = 0
  source_len = <blen>(source as .binary)
  scan_bytes = source as .binary
  call binresize scan_bytes, source_len + 1

/*!re2c
    re2c:yyfill:enable = 0;
    re2c:indent:top = 1;
    re2c:YYCURSOR = "pos";
    re2c:YYINPUT = "scan_bytes";
    re2c:yych = "byte";
    re2c:yystate = "state";
    re2c:state:abort = 0;
    re2c:nested-ifs = 1;
    re2c:bit-vectors = 0;

    space = [ \t\r\n];
    digit = [0-9];
    ident_start = [A-Za-z_];
    ident_continue = [A-Za-z0-9_];

    "\x00" {call level_l_re2c_emit tokens, count, TOK_EOF, source_len, 0, 0
      return tokens
    }
    space+ {start = pos
      state = 0
      iterate
    }
    digit+ {value = level_l_re2c_decimal(scan_bytes, start, pos - start)
      call level_l_re2c_emit tokens, count, TOK_NUMBER, start, pos - start, value
      start = pos
      state = 0
      iterate
    }
    ident_start ident_continue* {call level_l_re2c_emit tokens, count, TOK_IDENT, start, pos - start, 0
      start = pos
      state = 0
      iterate
    }
    "+" {call level_l_re2c_emit tokens, count, TOK_PLUS, start, 1, 0
      start = pos
      state = 0
      iterate
    }
    "-" {call level_l_re2c_emit tokens, count, TOK_MINUS, start, 1, 0
      start = pos
      state = 0
      iterate
    }
    "*" {call level_l_re2c_emit tokens, count, TOK_STAR, start, 1, 0
      start = pos
      state = 0
      iterate
    }
    "/" {call level_l_re2c_emit tokens, count, TOK_SLASH, start, 1, 0
      start = pos
      state = 0
      iterate
    }
    "(" {call level_l_re2c_emit tokens, count, TOK_LPAREN, start, 1, 0
      start = pos
      state = 0
      iterate
    }
    ")" {call level_l_re2c_emit tokens, count, TOK_RPAREN, start, 1, 0
      start = pos
      state = 0
      iterate
    }
    * {_level_l_status = 1
      _level_l_error_offset = start
      return tokens
    }
*/

level_l_re2c_status: procedure = .int expose _level_l_status _level_l_error_offset
  return _level_l_status

level_l_re2c_error_offset: procedure = .int expose _level_l_status _level_l_error_offset
  return _level_l_error_offset

level_l_re2c_token_count: procedure = .int
  arg tokens = .binary
  return <blen>(tokens) <idiv> TOKEN_SIZE

level_l_re2c_token_kind: procedure = .int
  arg tokens = .binary, index = .int
  return <at..u16>(index * TOKEN_SIZE + TOKEN_KIND) tokens

level_l_re2c_token_start: procedure = .int
  arg tokens = .binary, index = .int
  return <at..u32>(index * TOKEN_SIZE + TOKEN_START) tokens

level_l_re2c_token_length: procedure = .int
  arg tokens = .binary, index = .int
  return <at..u16>(index * TOKEN_SIZE + TOKEN_LENGTH) tokens

level_l_re2c_token_value: procedure = .int
  arg tokens = .binary, index = .int
  return <at..int>(index * TOKEN_SIZE + TOKEN_VALUE) tokens

level_l_re2c_token_name: procedure = .string
  arg kind = .int
  select kind
    when TOK_EOF then return "eof"
    when TOK_NUMBER then return "number"
    when TOK_IDENT then return "ident"
    when TOK_PLUS then return "plus"
    when TOK_MINUS then return "minus"
    when TOK_STAR then return "star"
    when TOK_SLASH then return "slash"
    when TOK_LPAREN then return "lparen"
    when TOK_RPAREN then return "rparen"
    otherwise return "unknown"
  end
  return "unknown"

level_l_re2c_emit: procedure = .int
  arg expose tokens = .binary, expose count = .int, kind = .int, start = .int, span_length = .int, value = .int
  offset = count * TOKEN_SIZE
  call binresize tokens, offset + TOKEN_SIZE
  <at..u16>(offset + TOKEN_KIND) tokens = kind
  <at..u32>(offset + TOKEN_START) tokens = start
  <at..u16>(offset + TOKEN_LENGTH) tokens = span_length
  <at..int>(offset + TOKEN_VALUE) tokens = value
  count = count + 1
  return count

level_l_re2c_decimal: procedure = .int
  arg source = .binary, start = .int, span_length = .int
  value = 0
  do index = start to start + span_length - 1
    value = value * 10 + <at..u8>(index) source - 48
  end
  return value
