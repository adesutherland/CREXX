/* Generated from authored Level L by the Level B bootstrap frontend. */
/* The retained re2c adapter constructs the DFA; this file is cREXX output. */
options levelb

namespace level_l_gennorm2 expose level_l_gennorm2_lex level_l_gennorm2_status level_l_gennorm2_error_offset level_l_gennorm2_error_message level_l_gennorm2_token_count level_l_gennorm2_token_kind level_l_gennorm2_token_start level_l_gennorm2_token_length level_l_gennorm2_token_name
import rxfnsb

level_l_gennorm2_lex: procedure = .binary expose _level_l_status _level_l_error_offset _level_l_error_message
  arg source = .string

  _level_l_status = 0
  _level_l_error_offset = 0
  _level_l_error_message = ""
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
    re2c:encoding:utf8 = 1;
    re2c:encoding-policy = fail;

    ascii_digit = [\u0030-\u0039];
    ascii_hex_letter = [\u0041-\u0046];
    horizontal_space = ("\u0009" | "\u0020");
    line_character = ([\u0000-\uD7FF\uE000-\U0010FFFF] \ (("\u0000" | "\u000A") | "\u000D"));
    hexadecimal_value = ((ascii_digit | ascii_hex_letter))+;
    version_text = ((ascii_digit)+ (("." (ascii_digit)+))+);
    line_end = (("\u000D")? "\u000A");
    comment_text = ("#" (line_character)*);

    (horizontal_space)+ {
      start = pos
      state = 0
      iterate
    }
    comment_text {
      start = pos
      state = 0
      iterate
    }
    "Unicode" {
      call level_l_gennorm2_emit tokens, count, 7, start, pos - start
      start = pos
      state = 0
      iterate
    }
    version_text {
      call level_l_gennorm2_emit tokens, count, 8, start, pos - start
      start = pos
      state = 0
      iterate
    }
    hexadecimal_value {
      call level_l_gennorm2_emit tokens, count, 1, start, pos - start
      start = pos
      state = 0
      iterate
    }
    ".." {
      call level_l_gennorm2_emit tokens, count, 2, start, pos - start
      start = pos
      state = 0
      iterate
    }
    ":" {
      call level_l_gennorm2_emit tokens, count, 3, start, pos - start
      start = pos
      state = 0
      iterate
    }
    "=" {
      call level_l_gennorm2_emit tokens, count, 4, start, pos - start
      start = pos
      state = 0
      iterate
    }
    ">" {
      call level_l_gennorm2_emit tokens, count, 5, start, pos - start
      start = pos
      state = 0
      iterate
    }
    "*" {
      call level_l_gennorm2_emit tokens, count, 6, start, pos - start
      start = pos
      state = 0
      iterate
    }
    line_end {
      call level_l_gennorm2_emit tokens, count, 9, start, pos - start
      start = pos
      state = 0
      iterate
    }
    "\u0000" {
      call level_l_gennorm2_emit tokens, count, 0, source_len, 0
      return tokens
    }
    * {
      _level_l_status = 2
      _level_l_error_offset = start
      _level_l_error_message = "invalid gennorm2 input character"
      return tokens
    }
*/

level_l_gennorm2_status: procedure = .int expose _level_l_status _level_l_error_offset _level_l_error_message
  return _level_l_status

level_l_gennorm2_error_offset: procedure = .int expose _level_l_status _level_l_error_offset _level_l_error_message
  return _level_l_error_offset

level_l_gennorm2_error_message: procedure = .string expose _level_l_status _level_l_error_offset _level_l_error_message
  return _level_l_error_message

level_l_gennorm2_token_count: procedure = .int
  arg tokens = .binary
  return <blen>(tokens) <idiv> 12

level_l_gennorm2_token_kind: procedure = .int
  arg tokens = .binary, index = .int
  return <at..u16>(index * 12) tokens

level_l_gennorm2_token_start: procedure = .int
  arg tokens = .binary, index = .int
  return <at..u32>(index * 12 + 2) tokens

level_l_gennorm2_token_length: procedure = .int
  arg tokens = .binary, index = .int
  return <at..u32>(index * 12 + 6) tokens

level_l_gennorm2_token_name: procedure = .string
  arg kind = .int
  select kind
    when 0 then return "eof"
    when 1 then return "hexadecimal"
    when 2 then return "range"
    when 3 then return "colon"
    when 4 then return "equals"
    when 5 then return "greater"
    when 6 then return "star"
    when 7 then return "unicode_keyword"
    when 8 then return "version"
    when 9 then return "newline"
    otherwise return "unknown"
  end
  return "unknown"

level_l_gennorm2_emit: procedure = .int
  arg expose tokens = .binary, expose count = .int, kind = .int, start = .int, span_length = .int
  offset = count * 12
  call binresize tokens, offset + 12
  <at..u16>(offset) tokens = kind
  <at..u32>(offset + 2) tokens = start
  <at..u32>(offset + 6) tokens = span_length
  count = count + 1
  return count
