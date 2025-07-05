# Comments

Commentary is included in a \crexx{} program by means of comments. \crexx{} has single-line comments and multiline[^block] comments.

[^block]: Also called block comments.

## Multiline comments

A multiline comment is started by a sequence of special characters ```/*```, and ended by the inverse character combination ```*/```
.
Within these delimiters any characters are allowed. Comments may be
nested, which is to say that ```/*``` and ```*/``` must pair correctly. Comments
may be anywhere, and may be of any length. They have no effect on the
program, except that they do act as separators (i.e., two tokens with just a
comment in between are not treated as a single token). These multiline comments do not need to straddle lines; it is ok to have them on one line only.

## Single line comments

For \crexx{} level B, the *octothorpe* (#), also know as poundsign or hashmark, is the default for single line comments. Other single line comment styles can be chosen by the ```option``` statements ```dashcomments``` and ```slashcomments``` which enable the SQL, \nr{} and Object \rexx{} ```--``` double dash style, and ```slashcomments``` which enable the C++ style ```//``` line comments.

Note: Although it is good practice to start a program with a comment that describes its purpose, this is not required for \crexx{} programs. On current platforms there is no need to distinguish \crexx{} programs from other languages for the purpose of the shell process.

## Summary


|option   |comment type   | symbol  |
|---|---|---|
| default |multiline |```/*``` ```*/```   | 
| default |single line |```#```   | 
| ```options dashcomment```  | single line  | ```--```  | 
| ```options slashcomment```  | single line  | ```//```  | 


Table: Comment options. {#tbl:id}


