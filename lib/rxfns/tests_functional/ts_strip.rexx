/* rexx */
options levelb

say "'"strip("   The quick brown fox jumps over the lazy dog  ")"'"
say "'"strip("   The quick brown fox jumps over the lazy dog  ",'L')"'"
say "'"strip("   The quick brown fox jumps over the lazy dog  ",'t')"'"
say "'"strip("-----The quick brown fox jumps over the lazy dog---",,'-')"'"
say "'"strip("-----The quick brown fox jumps over the lazy dog---",'B','-')"'"
say "'"strip("-----The quick brown fox jumps over the lazy dog---",'l','-')"'"
say "'"strip("-----The quick brown fox jumps over the lazy dog---",'t','-')"'"

return

/* strip()  */
strip: procedure = .string
  arg string1 = .string, option='B', tchar=" "

