/* Rexx Program to read the Unicode grapheme break tests and build a c test suite */

call build "GraphemeBreakTest-15.0.0.txt", "GraphemeBreakTest.c", "_grapheme"
call build "WordBreakTest-15.0.0.txt", "WordBreakTest.c", "_word"

exit

build: procedure
   parse arg in_file, out_file, suffix

   line_no = 0
   no_tests = 0

   "rm" out_file
   call lineout out_file, '/* Test Suite - Auto generated file! */'
   /* call lineout out_file, '#include <assert.h>' */
   call lineout out_file, '#include <stdio.h>'
   call lineout out_file, '#include <string.h>'
   call lineout out_file, ''
   call lineout out_file, 'void encodechar_utf32_8(unsigned int cp, char **buffer);'
   call lineout out_file, 'void append_to_buffer(char* to_append, char **buffer);'
   call lineout out_file, 'int lex'suffix'(char *str, char* out);'
   call lineout out_file, ''
   call lineout out_file, 'int tests'suffix'() {'
   call lineout out_file, '    char in_buffer[250];'
   call lineout out_file, '    char out_buffer[250];'
   call lineout out_file, '    char expected_buffer[250];'
   call lineout out_file, '    char* in;'
   call lineout out_file, '    char* expected;'
   call lineout out_file, '    int errors = 0;'
   call lineout out_file, ''

   do while lines(in_file)
      line = linein(in_file)
      line_no = line_no + 1

      if line = "" then iterate
      if left(strip(line),1) = "#" then iterate
      parse var line tests "#" comment
      if tests = "" then iterate
      tests = strip(tests)
      comment = strip(comment)

      no_tests = no_tests + 1

      /* Note that ÷ is 2 bytes in utf8 so this code is meant to work for non-utf8 aware rexx processors */
      if word(tests, 1) \= "÷" then do
         say "Line" line_no "I only understand tests starting with ÷"
         exit 1
      end
      if word(tests, words(tests)) \= "÷" then do
         say "Line" line_no "I only understand tests ending with ÷"
         exit 1
      end
      tests = strip(right(tests, length(tests) - length("÷")))

      /* Start test */
      call lineout out_file, '    // TEST' no_tests

      /* Test Input */
      call lineout out_file, '    // Input'
      call lineout out_file, '    in = in_buffer;'
      t = tests
      do while t \= ""
         parse var t codepoint break t
         call lineout out_file, '    encodechar_utf32_8(0x'codepoint', &in);'
      end
      call lineout out_file, '    *in = 0;'

      /* Test Expected Output */
      call lineout out_file, '    // Expected output'
      call lineout out_file, '    expected = expected_buffer;'
      call lineout out_file, '    append_to_buffer("[",&expected);'
      t = tests
      do while t \= ""
         parse var t codepoint break t
         call lineout out_file, '    encodechar_utf32_8(0x'codepoint', &expected);'
         if break = "÷" then do
            call lineout out_file, '    append_to_buffer("]",&expected);'
            if t \= "" then call lineout out_file, '    append_to_buffer("[",&expected);'
         end
         else if break \= "×" then do
             say "Line" line_no "I only understand tests breaks with ÷ or ×"
             exit 1
         end
      end
      call lineout out_file, '    *expected = 0;'

      /* Execute and Report if test failure */
      call lineout out_file, '    lex'suffix'(in_buffer, out_buffer);'
      call lineout out_file, '    if (strcmp(out_buffer, expected_buffer) != 0) {'
      call lineout out_file, '        errors++;'
      call lineout out_file, '        printf("*** ERROR ***\n");'
      call lineout out_file, '        printf("TEST     'no_tests '\"'comment'\"\n");'
      call lineout out_file, '        printf("INPUT    \"%s\"\n", in_buffer);'
      call lineout out_file, '        printf("EXPECTED \"%s\"\n", expected_buffer);'
      call lineout out_file, '        printf("OUTPUT   \"%s\"\n", out_buffer);'
      call lineout out_file, '        printf("\n");'
      call lineout out_file, '    }'
      call lineout out_file, ''

   end
   call lineout in_file

   call lineout out_file, '    printf("'no_tests' tests run\n");'
   call lineout out_file, '    if (errors) printf("*** %d ERRORS ***\n", errors);'
   call lineout out_file, '    return errors;'
   call lineout out_file, '}'
   call lineout out_file, ''
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