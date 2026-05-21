options levelb
import rxfnsb

import scanlex

main: procedure
say "Test 1"
say "======"
call ScanLexRun 'a = left(name,10) "Please add my telephone number" if true then cmp=false'

say " "
say "Test 2"
say "======"
call ScanLexRun "if total >= 10.5 then say 'That''s fine' || left(name.1, 8)"

say " "
say "Test 3"
say "======"
call ScanLexRun "x=-12; y=3.14E+2; z='a''b'; a\=b"

say " "
say "Test 4"
say "======"
call ScanLexRun "x=-12; y=3.14E+2; z='a''b'; a\=b"

say " "
say "Test 5"
say "======"
call ScanLexRun "x=-12; y=3.14E+2; z='a''b'; a\=b"

say " "
say "Test 6"
say "======"
call ScanLexRun "x=-12; /* now we test scientific numbers */ y=3.14E+2; z='a''b'; a\=b -- this is a line end comment"

return

ScanLexRun: procedure
/* -------------------------------------------------------------------------------------------------------------------
 * scanlex_demo.rexx
 *
 * Demonstrates how to use the SCANLEX tokenizer.
 *
 * The program:
 *   1. Tokenizes a Rexx expression.
 *   2. Prints each token with:
 *        - index
 *        - type
 *        - text
 *        - source position
 *   3. Counts tokens by category.
 *   4. Shows how to search for specific token types.
 *
 * Usage:
 *   crexx scanlex_demo.rexx
 * -------------------------------------------------------------------------------------------------------------------
 */
  arg text=.string

  say "Input: "text
  say

  /* Tokenize the source string. */
  tokens = rxScanLex(text)

  say copies("-", 90)
  say left("No", 5) left("Type", 20)'  'left("Pos", 4)"   Text"
  say copies("-", 90)

  do i = 1 to tokens.0
     tok = tokens[i]
     say left(i, 5),
         left(tok.get_type(), 20),
         right(tok.get_pos(),4)'   ',
         tok.get_text()
  end

  say copies("-", 90)
  say "Total tokens:" tokens.0
  say

  /* Count token types. */
  identifier_count = 0
  literal_count    = 0
  operator_count   = 0
  comment_count    = 0

  do i = 1 to tokens.0
     tok = tokens[i]
     type = tok.get_type()

     select
        when type = "identifier" then
           identifier_count = identifier_count + 1

        when type = "int_literal" | ,
             type = "decimal_literal" | ,
             type = "string_literal" then
           literal_count = literal_count + 1

        when type = "operator" then
           operator_count = operator_count + 1

        when type = "comment" then
           comment_count = comment_count + 1
     end
  end

  say "Statistics:"
  say "  Identifiers :" identifier_count
  say "  Literals    :" literal_count
  say "  Operators   :" operator_count
  say "  Comments    :" comment_count
  say

  /* Example: find all identifiers. */
  say "Identifiers found:"
  do i = 1 to tokens.0
     tok = tokens[i]
     if tok.get_type() = "identifier" then
        say "  " tok.get_text()
  end
  say

  /* Example: detect function calls.
   * If an identifier is followed by "(" it is likely a function call.
   */
  say "Possible function calls:"
  fi=0
  do i = 1 to tokens.0 - 1
     if tokens[i].get_type() = "identifier" & ,
        tokens[i + 1].get_type() = "bracket" & ,
        tokens[i + 1].get_text() = "(" then do
           fi=fi+1
           say "  " tokens[i].get_text()
        end
  end
  if fi=0 then say "   None"
  say copies("-", 90)
return