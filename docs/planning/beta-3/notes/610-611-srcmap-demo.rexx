/* Source-map algorithm demo for beta 3 issues 610 and 611.
 *
 * This is a design aid, not production RXPP or rxc code. It demonstrates:
 * - RXPP-style marker emission for one macro-expanded line;
 * - rxc-style marker stripping and generated-span capture;
 * - remapping generated diagnostics back to the original source span.
 */
options levelb
import rxfnsb

source_file = "demo.rxpp"
source_line_number = 3
source_line = "answer = SQUARE(totl + 1)"
source_col_base = pos("SQUARE", source_line)

quote = '"'
file_directive = "@" || quote || escape_payload(source_file) || quote
line_directive = "@" || source_line_number || "l" || quote || escape_payload(source_line) || quote
column_directive = "@" || source_col_base || "c"

/* RXPP knows the macro call starts at source_col_base. The inner token totl is
 * seven columns after that anchor, so relative spans keep emission simple.
 */
generated = "answer = @+0+16{(@+7+4{totl@} + 1) * (@+7+4{totl@} + 1)@}"

say "RXPP emits:"
say "options levelb srcmap"
say file_directive
say line_directive
say column_directive
say generated
say ""

clean = ""
i = 1
map_count = 0
stack_count = 0

map_gen_start = .int[]
map_gen_len = .int[]
map_src_col = .int[]
map_src_len = .int[]
stack_gen_start = .int[]
stack_src_col = .int[]
stack_src_len = .int[]

do while i <= length(generated)
  ch = substr(generated, i, 1)

  if ch <> "@" then do
    clean = clean || ch
    i = i + 1
    iterate
  end

  next = substr(generated, i + 1, 1)

  if next = "@" then do
    clean = clean || "@"
    i = i + 2
    iterate
  end

  if next = "}" then do
    map_count = map_count + 1
    map_gen_start[map_count] = stack_gen_start[stack_count]
    map_gen_len[map_count] = length(clean) - stack_gen_start[stack_count] + 1
    map_src_col[map_count] = stack_src_col[stack_count]
    map_src_len[map_count] = stack_src_len[stack_count]
    stack_count = stack_count - 1
    i = i + 2
    iterate
  end

  /* Parse @+offset+length{ or @column+length{. This demo only needs column
   * maps; file, line, and column-base directives are shown above.
   */
  scan = i + 1
  relative = 0
  if substr(generated, scan, 1) = "+" then do
    relative = 1
    scan = scan + 1
  end

  number_start = scan
  do while scan <= length(generated) & substr(generated, scan, 1) <> "+"
    scan = scan + 1
  end
  column_text = substr(generated, number_start, scan - number_start)

  scan = scan + 1
  length_start = scan
  do while scan <= length(generated) & substr(generated, scan, 1) <> "{"
    scan = scan + 1
  end
  span_len_text = substr(generated, length_start, scan - length_start)

  stack_count = stack_count + 1
  stack_gen_start[stack_count] = length(clean) + 1
  if relative = 1 then stack_src_col[stack_count] = source_col_base + column_text
  else stack_src_col[stack_count] = column_text
  stack_src_len[stack_count] = span_len_text

  i = scan + 1
end

say "rxc compiles cleaned generated source:"
say clean
say ""

say "rxc captured maps:"
do m = 1 to map_count
  say " generated col" map_gen_start[m] "len" map_gen_len[m] "-> source col" map_src_col[m] "len" map_src_len[m]
end
say ""

call remap_and_show "unknown variable totl", 24, map_count, map_gen_start, map_gen_len, map_src_col, map_src_len, clean, source_file, source_line_number, source_line
call remap_and_show "diagnostic on generated operator", 21, map_count, map_gen_start, map_gen_len, map_src_col, map_src_len, clean, source_file, source_line_number, source_line

return

escape_payload: procedure = .string
  arg text = .string
  quote = '"'
  out = ""
  do i = 1 to length(text)
    ch = substr(text, i, 1)
    if ch = "@" then out = out || "@@"
    else if ch = quote then out = out || quote || quote
    else out = out || ch
  end
  return out

remap_and_show: procedure
  arg message = .string, gen_error_col = .int, map_count = .int, map_gen_start = .int[], map_gen_len = .int[], map_src_col = .int[], map_src_len = .int[], clean = .string, source_file = .string, source_line_number = .int, source_line = .string

  best = 0
  best_len = 1000000
  do m = 1 to map_count
    if gen_error_col >= map_gen_start[m] & gen_error_col < map_gen_start[m] + map_gen_len[m] then do
      if map_gen_len[m] < best_len then do
        best = m
        best_len = map_gen_len[m]
      end
    end
  end

  say "generated diagnostic:"
  say clean
  gen_caret = ""
  do j = 1 to gen_error_col - 1
    gen_caret = gen_caret || " "
  end
  say gen_caret || "^ " || message

  if best = 0 then do
    say "no source-map span found"
    return
  end

  say "remapped diagnostic:"
  say source_file || ":" || source_line_number || ":" || map_src_col[best] || ": " || message
  say source_line
  src_caret = ""
  do j = 1 to map_src_col[best] - 1
    src_caret = src_caret || " "
  end
  do j = 1 to map_src_len[best]
    src_caret = src_caret || "^"
  end
  say src_caret
  say ""
  return
