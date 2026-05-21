options levelb
import rxfnsb

import scanlex

main: procedure
say "JSON Test 1"
say "==========="

call scanJSONRun '{"id":1001,"user":{"name":"Peter","roles":["admin","developer"],"enabled":true},"metrics":{"score":-42,"ratio":0.875,"growth":-1.25E-3},"items":[{"sku":"A-100","qty":2},{"sku":"B-200","qty":0}],"tags":[],"note":null}'

return

ScanJSONRun: procedure
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
  tokens = rxScanJSON(text)

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
  say copies("-", 90)
return