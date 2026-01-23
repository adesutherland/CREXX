/* Test Import - Direct Symbol Resolution */
options levelb
import mymath

/* Direct call to the imported function (no namespace prefix needed/supported yet) */
say myfunc(10)
