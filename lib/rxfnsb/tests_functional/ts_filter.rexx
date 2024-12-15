/* rexx */
options levelb
import rxfnsb

say filter('the quick fox jumps over the lazy brown dog',' ')
say filter('the-+quick*fox/_jumps over...the\lazy brown#dog','-+*/\. _#')
