/* --- Standard Macro Library for RXPP --- */

##define mainargs(stem)    {arg stem=.string[]}
##define strlen(len,strg)  {len=0; assembler strlen len,strg}

/* Math Helpers */
##define CUBE(x)           {x*x*x}
##define SQUARE(x) 	       {x*x}
##define double(x)   	   {2*x}

/* Data Handling */
##define hi(stem)          {stem.0}
##define mapput(map,k,v)   {map.k=v}
##define mapget(map,k)     {map.k}
##define quote(string2quote)  {'string2quote'}
##define Dquote(string2quote) {"string2quote"}

/* Control Flow Shortcuts */
##define repeat(n)         {do __i=1 to n}
##define guard(cond,act)   {if cond<>0 then do; act; return; end}

/* Variadic & Utility */
##define SetStem(name, ...)  {name.$indx=arglist.$indx}

##define foreach(stem,indx)   {do indx=1 to stem.0 }
##define forpair(array, key, val)  {do key=1 to array.0; val=array.key}

/* System & Timing */

/* Miscellaneous */
##define swap(a,b)         {temp=a; a=b; b=temp}

/* --- File Helpers --- */
##define readfile(stem, file)   {stem.1='' ; call readall stem,file,-1}
##define writefile(file, stem) {call writeall(file, stem, 1, stem.0)}

/* --- String Functions --- */
##define ltrim(str)    {strip(str, 'L')}
##define rtrim(str)    {strip(str, 'T')}

##define startswith(suffix,string)    {substr(string, 1, length(prefix)) = prefix}
##define endswith(suffix,string)      {substr(string, length(string) - length(suffix) + 1) = suffix}

##define isDigit(string)      {verify(string, '0123456789') = 0}
##define isAlpha(string)      {verify(string, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz') = 0}
##define isBlank(str)         {verify(str, ' ') = 0}
##define isUpper(str)         {verify(str, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ') = 0}
##define isLower(str)         {verify(str, 'abcdefghijklmnopqrstuvwxyz') = 0}
##define isAlnum(str)         {verify(str, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') = 0}
##define isSpace(str)         {verify(str, ' ') = 0}
##define isEmpty(str)         {str = ''}
##define notEmpty(str)        {str \= ''}
##define isPunct(str)         {verify(str, ',.;:!?()[]{}+-*/=<>@#$%^&|~') = 0}
##define charAt(str, pos)     {substr(str, pos, 1)}

##define isLonger(str, len)   {length(str) > len}
##define isShorter(str, len)  {length(str) < len}
##define equals(a, b)         {a = b}
##define equalsFold(a,b)      {translate(a) = translate(b)}

##define contains(str, sub)     {pos(sub, str) > 0}

/* Numerical Functions */
##define isEven(n)          {n // 2 = 0}
##define isOdd(n)           {n // 2 \= 0}
##define isPositive(n)      {n > 0}
##define isNegative(n)      {n < 0}
##define isZero(n)          {n = 0}

##define clamp(val,lo,hi)   {val < lo & lo | val > hi & hi | val}

##define between(n,a,b)     {n >= a & n <= b}
##define inRange(n,a,b)     {between(n,a,b)} /* Alias */

##define isLeapYear(y)      {(y // 4 = 0) & (y // 100 \= 0) | (y // 400 = 0)}

/* --- Mathematical Macros --- */
##define sign(n)            {n > 0 & 1 | n < 0 & -1 | 0}
##define pi()               {3.141592653589793}
##define euler()            {2.718281828459045}

/* --- Logical an Comparisons --- */
##define ifNull(val,fallback) {val\='' & val | fallback}
##define xor(a,b)           {(a | b) & \(a & b)}

/* --- Debug & Utilities --- */
##define debug(expr)       {say '>>' expr '=' expr}
##define comment(msg)      {/* msg */}
##define traceblock(name)  {say '--- enter ' name}

##define log(msg)           {say time('l') msg}
##define traceValue(v)      {say '→' v}
##define info(msg)          {say 'INFO:    ' msg}
##define error(msg)         {say 'ERROR:   ' msg}
##define warn(msg)          {say 'WARNING: ' msg}