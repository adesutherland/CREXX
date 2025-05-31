/* --- Standard Macro Library for RXPP --- */

/* Debugging & Introspection */
##define debug(expr)       {say '>>' expr '=' expr}
##define comment(msg)      {/* msg */}
##define traceblock(name)  {say '--- enter ' name}

/* Math Helpers */
##define CUBE(x)           {x*x*x}
##define SQUARE(x) 		   {x*x}
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
##define list2Stem(name, ...)  {name.$indx=arglist.$indx}

##define foreach(stem,indx)   {do indx=1 to stem.0 }
##define forpair(array, key, val)  {do key=1 to array.0; val=array.key}

/* System & Timing */
##define benchmark(start,end)  {say 'Elapsed:' (end-start) 'ms'}

/* Miscellaneous */
##define swap(a,b)         {temp=a; a=b; b=temp}
##define ifdef(var, code)  {if symbol(var) = 'VAR' then do; code; end}

/* --- File Helpers --- */
##define readfile(stem, file)   {stem.1='' ; call readall stem,file,-1}
##define writefile(file, stem) {call writeall(file, stem, 1, stem.0)}
##define ltrim(str)    {strip(str, 'L')}
##define rtrim(str)    {strip(str, 'T')}




