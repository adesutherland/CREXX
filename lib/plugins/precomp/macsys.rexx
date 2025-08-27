/* --- System Macro Library for RXPP, change only if supervised --- */
##define log(msg)           {say time('l') msg}

##define mainargs(stem)    {arg stem=.string[]}
##define strlen(len,strg)  {len=0; assembler strlen len,strg}

/* Numerical Functions */
##define isEven(n)          {n // 2 = 0}
##define isOdd(n)           {n // 2 \= 0}
##define isPositive(n)      {n > 0}
##define isNegative(n)      {n < 0}
##define isZero(n)          {n = 0}

/* --- Mathematical Macros --- */
##define sign(n)            {n > 0 & 1 | n < 0 & -1 | 0}
##define pi()               {3.141592653589793}
##define euler()            {2.718281828459045}

/* --- Logical an Comparisons --- */
##define ifNull(val,fallback) {val\='' & val | fallback}
##define xor(a,b)           {(a | b) & \(a & b)}

##define info(msg)          {say 'INFO:    ' msg}
##define error(msg)         {say 'ERROR:   ' msg}
##define warn(msg)          {say 'WARNING: ' msg}

##define isDigit(string)      {verify(string, '0123456789') = 0}
##define isAlpha(string)      {verify(string, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz') = 0}
##define isBlank(str)         {verify(str, ' ') = 0}
##define isUpper(str)         {verify(str, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ') = 0}
##define isLower(str)         {verify(str, 'abcdefghijklmnopqrstuvwxyz') = 0}
##define isAlnum(str)         {verify(str, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') = 0}
##define isSpace(str)         {verify(str, ' ') = 0}
##define isEmpty(str)         {str = ''}