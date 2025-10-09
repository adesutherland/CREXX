/* --- Standard Macro Library for RXPP --- */

/* Math Helpers */
##define CUBE(x)           {x*x*x}
##define SQUARE(x) 	       {x*x}
##define double(x)   	   {2*x}

/* Data Handling */
##define hi(stem)          {stem.0}
##define mapput(map,k,v)   {map.k=v}
##define mapget(map,k)     {map.k}

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

##define notEmpty(str)        {str \= ''}
##define isPunct(str)         {verify(str, ',.;:!?()[]{}+-*/=<>@#$%^&|~') = 0}
##define charAt(str, pos)     {substr(str, pos, 1)}

##define isLonger(str, len)   {length(str) > len}
##define isShorter(str, len)  {length(str) < len}
##define equals(a, b)         {a = b}
##define equalsFold(a,b)      {translate(a) = translate(b)}

##define contains(str, sub)     {pos(sub, str) > 0}

##define clamp(val,lo,hi)   {val < lo & lo | val > hi & hi | val}

##define between(n,a,b)     {n >= a & n <= b}
##define inRange(n,a,b)     {between(n,a,b)} /* Alias */

##define isLeapYear(y)      {(y % 4 = 0) & (y % 100 \= 0) | (y % 400 = 0)}

/* --- Debug & Utilities --- */
##define debug(expr)       {say '>>' expr '=' expr}
##define comment(msg)      {/* msg */}
##define traceblock(name)  {say '--- enter ' name}

##define traceValue(v)      {say '→' v}

##define cparse(string,template)  \
               {_pass_variable.1='' ; _pass_variable_content.1='' \
                string2Parse=string; parsetemplate=template\
                call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content \
               }
