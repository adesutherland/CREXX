/* Rexx Program to read the Unicode Character Databases and build re2c compatible headers with
 * character classifications */

say "Unicode Character Database to re2c converter"
say "=========================================="
say "This program reads the Unicode Character Database and builds re2c compatible headers with"
say "character classifications."
say
say "The Unicode Character Database is available at:"
say "https://www.unicode.org/Public/UCD/latest/ucd/"
say
say "The re2c tool is available at:"
say "https://re2c.org/"
say
say "The generated files are:"
say "utfcharbreak.re - Grapheme Break Property"
say "utfwordbreak.re - Word Break Property"
say "emoji-data.re - Emoji Data"
say
say "The generated files are placed in the current directory."
say
say "The program will now download the Unicode Character Database and build the files."
say "This may take a while."
say

/* Download the Unicode Character Database */
"curl -O https://www.unicode.org/Public/UCD/latest/ucd/GraphemeBreakProperty.txt"
"curl -O https://www.unicode.org/Public/UCD/latest/ucd/emoji-data.txt"
"curl -O https://www.unicode.org/Public/UCD/latest/ucd/WordBreakProperty.txt"

/* GraphemeBreakProperty-15.0.0.txt */
call build "GraphemeBreakProperty.txt", "utfcharbreak.re"
call build "emoji-data.txt", "emoji-data.re"
call build "WordBreakProperty.txt", "utfwordbreak.re"

exit

build: procedure
   parse arg in_file, out_file

   types.0 = 0
   type_index. = 0
   type_codes. = ""

   do while lines(in_file)
      line = linein(in_file)
      if line = "" then iterate
      if left(strip(line),1) = "#" then iterate
      parse var line code ";" type "#"
      if type = "" then iterate
      parse var code code1 ".." code2
      code1 = strip(strip(code1),"l","0")
      code2 = strip(strip(code2),"l","0")
      type = strip(type)

      len = max(length(code1),length(code2))

      if code2 = "" then addr = escape(code1,len)
      else addr = escape(code1,len)"-"escape(code2,len)

      ix = type_index.type
      if ix = 0 then do
        ix = types.0 + 1
        types.0 = ix
        type_index.type = ix
        types.ix = type
      end

      type_codes.ix = type_codes.ix || addr
   end
   call lineout in_file

   "rm" out_file
   call lineout out_file, "/*!re2c"
   do ix = 1 to types.0
      call lineout out_file, types.ix "= ["type_codes.ix"];"
   end
   call lineout out_file, "*/"
   call lineout out_file

   return

escape: procedure
   parse arg code, len

   if len<=2 then len=2
   else if len<=4 then len=4
   else if len<=8 then len=8

   code = right(lower(code), len, "0")
   select
      when len=2 then code = "\x"code
      when len=4 then code = "\u"code
      otherwise code = "\U"code
   end

   return code