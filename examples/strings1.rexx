/* strings.rexx
   CREXX provides the classic REXX string functions as well as
   quote-aware Q* variants. The Q* functions treat quoted text
   as a single word, which is useful when parsing command lines.
*/
options levelb
import rxfnsb

text = 'The quick brown fox '

say 'Classic REXX String Functions'
say '-----------------------------'
say 'Original :' text
say 'Length   :' length(text)           /* Number of characters           */
say 'Words    :' words(text)            /* Number of blank-separated words */
say 'Word 3   :' word(text, 3)          /* Third word: brown              */
say 'Upper    :' translate(text)        /* Convert to uppercase           */
say 'Reverse  :' reverse(text)          /* Reverse character order        */
say 'Substr   :' substr(text, 5, 5)     /* Five characters starting at 5  */
say 'Pos fox  :' pos('fox', text)       /* Position of "fox"              */
say 'Strip    :' strip('  hello  ')     /* Remove leading/trailing blanks */

say
say 'Quote Aware String Functions'
say '----------------------------'

text = 'The "quick brown" fox jumps over the lazy dog'

say 'Original   :' text
say 'Length     :' length(text)
say 'QWords     :' qwords(text)         /* "quick brown" counts as one word */
say 'QWord 2    :' qword(text, 2)       /* Returns: quick brown           */
say 'QWord 3    :' qword(text, 3)       /* Returns: fox                   */
say 'QPos fox   :' qpos('fox', text)    /* Position of "fox"              */
say 'QPos brown :' qpos('brown', text)  /* Position inside quoted text    */