# ASCII research contract: negative ids skip trivia; order breaks ties.
-1 [ \t\r\n]+
-2 /\*([^*]|\*+[^*/])*\*+/
-3 //[^\r\n]*
1 [A-Za-z_][A-Za-z_0-9]*
2 [0-9]+
3 '([^'\r\n]|'')*'
4 \+
5 \*
6 \(
7 \)
8 =
9 ;
10 ,
11 \[
12 \]
