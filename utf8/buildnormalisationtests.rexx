/* Rexx Program to read the Unicode normalisation tests and build a c test suite */

call build "NormalizationTest.txt", "NormalizationTest.c"

exit

build: procedure
   parse arg in_file, out_file

   line_no = 0
   no_tests = 0

   "rm" out_file
   call lineout out_file, '/* Test Suite - Auto generated file! */'
   /* call lineout out_file, '#include <assert.h>' */
   call lineout out_file, '#include <stdio.h>'
   call lineout out_file, '#include <string.h>'
   call lineout out_file, '#include <stdlib.h>'
   call lineout out_file, ''
   call lineout out_file, 'int nfd_normaliser(const char *str, int len, char* out, int *out_len);'
   call lineout out_file, 'int nfkd_normaliser(const char *str, int len, char* out, int *out_len);'
   call lineout out_file, 'int nfc_normaliser(const char *str, int len, char* out, int *out_len);'
   call lineout out_file, 'int nfkc_normaliser(const char *str, int len, char* out, int *out_len);'
   call lineout out_file, ''
   call lineout out_file, 'static int errors = 0;'
   call lineout out_file, ''
   call lineout out_file, '/* Function to convert a utf8 string to a list of code points */'
   call lineout out_file, 'char* utf8_to_codepoints(const char *utf8_str) {'
   call lineout out_file, '    unsigned char *ptr = (unsigned char *)utf8_str;'
   call lineout out_file, '    int code_point = 0;'
   call lineout out_file, '    int num_bytes = 0;'
   call lineout out_file, '    int i;'
   call lineout out_file, '    char buffer[10];  // Buffer for storing individual codepoints as strings'
   call lineout out_file, '    static char output[1000];  // Buffer for storing the output string'
   call lineout out_file, '    output[0] = 0;'
   call lineout out_file, ''
   call lineout out_file, '    while (*ptr) {'
   call lineout out_file, '        // Initialize code_point and num_bytes for each new character'
   call lineout out_file, '        code_point = 0;'
   call lineout out_file, '        num_bytes = 0;'
   call lineout out_file, ''
   call lineout out_file, '        // Determine the number of bytes for this character'
   call lineout out_file, '        if ((*ptr & 0x80) == 0) {        // 1-byte (ASCII)'
   call lineout out_file, '            code_point = *ptr;'
   call lineout out_file, '            num_bytes = 1;'
   call lineout out_file, '        } else if ((*ptr & 0xE0) == 0xC0) { // 2-bytes'
   call lineout out_file, '            code_point = *ptr & 0x1F;'
   call lineout out_file, '            num_bytes = 2;'
   call lineout out_file, '        } else if ((*ptr & 0xF0) == 0xE0) { // 3-bytes'
   call lineout out_file, '            code_point = *ptr & 0x0F;'
   call lineout out_file, '            num_bytes = 3;'
   call lineout out_file, '        } else if ((*ptr & 0xF8) == 0xF0) { // 4-bytes'
   call lineout out_file, '            code_point = *ptr & 0x07;'
   call lineout out_file, '            num_bytes = 4;'
   call lineout out_file, '        }'
   call lineout out_file, ''
   call lineout out_file, '        // Read the remaining bytes for this character'
   call lineout out_file, '        for (i = 1; i < num_bytes; i++) {'
   call lineout out_file, '            ptr++;'
   call lineout out_file, '            if ((*ptr & 0xC0) == 0x80) {  // Continuation byte'
   call lineout out_file, '                code_point = (code_point << 6) | (*ptr & 0x3F);'
   call lineout out_file, '            }'
   call lineout out_file, '        }'
   call lineout out_file, ''
   call lineout out_file, '        // Append the code point to the output string'
   call lineout out_file, '        sprintf(buffer, "%04X ", code_point);'
   call lineout out_file, '        strcat(output, buffer);'
   call lineout out_file, ''
   call lineout out_file, '        // Move to the next character'
   call lineout out_file, '        ptr++;'
   call lineout out_file, '    }'
   call lineout out_file, '    return output;'
   call lineout out_file, '}'
   call lineout out_file, ''
   call lineout out_file, '/* Function to check result */'
   call lineout out_file, 'void checkresult(int rc, char* input, char* expected, char* output, char* test_id) {'
   call lineout out_file, '    if (rc > 0 || strcmp(output, expected) != 0) {'
   call lineout out_file, '        errors++;'
   call lineout out_file, '        printf("*** ERROR ***\n");'
   call lineout out_file, '        printf("TEST     %s\n", test_id);'
   call lineout out_file, '        printf("RC       %d\n", rc);'
   call lineout out_file, '        printf("INPUT    \"%s\" ( %s)\n", input, utf8_to_codepoints(input));'
   call lineout out_file, '        printf("EXPECTED \"%s\" ( %s)\n", expected, utf8_to_codepoints(expected));'
   call lineout out_file, '        printf("OUTPUT   \"%s\" ( %s)\n", output, utf8_to_codepoints(output));'
   call lineout out_file, '        printf("\n");'
   call lineout out_file, '    }'
   call lineout out_file, '    return;'
   call lineout out_file, '}'
   call lineout out_file, ''
   call lineout out_file, 'int main() {'
   call lineout out_file, '    int out_len;'
   call lineout out_file, '    char out_buffer[250];'
   call lineout out_file, '    int err = 0;'
   call lineout out_file, ''

   part = ""
   do while lines(in_file)
      line = linein(in_file)
      line_no = line_no + 1

      if line = "" then iterate
      if left(strip(line),1) = "#" then iterate
      if left(strip(line),1) = "@" then do
         parse upper var line "@" part "#" comment
         part = strip(part) "-" strip(comment)
         iterate
      end
      parse var line source ";" NFC ";" NFD ";" NFKC ";" NFKD ";" "#" comment
      if source = "" then iterate

      source = to_utf8_string(source)
      NFC = to_utf8_string(NFC)
      NFD = to_utf8_string(NFD)
      NFKC = to_utf8_string(NFKC)
      NFKD = to_utf8_string(NFKD)
      comment = strip(comment)

      /* Execute test */
      no_tests = no_tests + 1;
      call lineout out_file, '    err = nfd_normaliser('source', strlen('source'), out_buffer, &out_len); out_buffer[out_len] = 0;'
 /*     call lineout out_file, '    printf("NFD   \"%s\" ( %s)\n", out_buffer, utf8_to_codepoints(out_buffer));' */
      call lineout out_file, '    checkresult(err, 'source', 'NFD', out_buffer, "'no_tests 'NFD \"'escape(comment)'\"");'
      call lineout out_file, ''

      no_tests = no_tests + 1;
      call lineout out_file, '    err = nfkd_normaliser('source', strlen('source'), out_buffer, &out_len); out_buffer[out_len] = 0;'
/*      call lineout out_file, '    printf("NFKD   \"%s\" ( %s)\n", out_buffer, utf8_to_codepoints(out_buffer));' */
      call lineout out_file, '    checkresult(err, 'source', 'NFKD', out_buffer, "'no_tests 'NFKD \"'escape(comment)'\"");'
      call lineout out_file, ''

      no_tests = no_tests + 1;
      call lineout out_file, '    err = nfc_normaliser('source', strlen('source'), out_buffer, &out_len); out_buffer[out_len] = 0;'
/*      call lineout out_file, '    printf("NFC   \"%s\" ( %s)\n", out_buffer, utf8_to_codepoints(out_buffer));' */
      call lineout out_file, '    checkresult(err, 'source', 'NFC', out_buffer, "'no_tests 'NFC \"'escape(comment)'\"");'
      call lineout out_file, ''
/*
      no_tests = no_tests + 1;
      call lineout out_file, '    err = nfkc_normaliser('source', strlen('source'), out_buffer, &out_len); out_buffer[out_len] = 0;'
 /*     call lineout out_file, '    printf("NFKC   \"%s\" ( %s)\n", out_buffer, utf8_to_codepoints(out_buffer));' */
      call lineout out_file, '    checkresult(err, 'source', 'NFKC', out_buffer, "'no_tests 'NFKC \"'escape(comment)'\"");'
      call lineout out_file, ''
*/

     if no_tests>1000 then leave

   end
   call lineout in_file

   call lineout out_file, '    printf("'no_tests' tests run\n");'
   call lineout out_file, '    if (errors) printf("*** %d ERRORS ***\n", errors);'
   call lineout out_file, '    else printf("*** NO ERRORS ***\n");'
   call lineout out_file, '    return errors;'
   call lineout out_file, '}'
   call lineout out_file, ''

   call lineout out_file

   return

/* Procedure to escape a string for C source */
escape: procedure
    parse arg string
    result = ""
    do i = 1 to length(string)
        ch = substr(string, i, 1)
        if ch = '"' then result = result '\"'
        else if ch = '\' then result = result "\\"
        else result = result || ch
    end
    return result

/* Procedure to convert a code_point to a utf-8 escaped string literal for C source */
to_utf8: Procedure
    parse arg code_point
    code_point = x2d(code_point)
    if code_point < 128 then
        return '"\x'd2x(code_point,2)'"'
    if code_point < 2048 then
        return '"\x'd2x(192 + (code_point % 64),2)'\x'd2x(128 + (code_point // 64),2)'"'
    if code_point < 65536 then
        return '"\x'd2x(224 + (code_point % 4096),2)'\x'd2x(128 + ((code_point % 64) // 64),2)'\x'd2x(128 + (code_point // 64),2)'"'
    if code_point < 2097152 then
        return '"\x'd2x(240 + (code_point % 262144),2)'\x'd2x(128 + ((code_point % 4096) // 64),2)'\x'd2x(128 + ((code_point % 64) // 64),2)'\x'd2x(128 + (code_point // 64),2)'"'
    return ""

/* Procedure to covert a string made up of space delimited codepoints to a utf-8 escaped string literal for C source */
to_utf8_string: procedure
    parse arg utf8_string
    result = ""
    do i = 1 to words(utf8_string)
        result = result to_utf8(word(utf8_string, i))
    end
    return space(result)
