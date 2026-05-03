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

## Doc (Documentation) Comments

*Doc comments* are a special form of multiline comment which starts with `/**` and ends in `*/`. It is not processed by the \crexx{} compiler but can aid in the producing of documentation by external tools. In doc commments special tags as `@parm`, `@result` and `@author` can be used, purely for documentation purposes and with no consequence for the behaviour of the program.

## Single line comments

For \crexx{} level B, the *octothorpe* (#), also know as poundsign or hashmark, is the default for single line comments. Other single line comment styles can be chosen by the ```option``` statements ```comments_dash``` and ```comments_slash``` which enable the SQL, \nr{} and Object \rexx{} ```--``` double dash style, and ```comments_slash``` which enable the C++ style ```//``` line comments.

Note: Although it is good practice to start a program with a comment that describes its purpose, this is not required for \crexx{} programs. On current platforms there is no need to distinguish \crexx{} programs from other languages for the purpose of the shell process.

## Summary

|option   |comment type   | symbol  |
|---|---|---|
| default |multiline |```/*``` ```*/```   | 
| default |single line |```#```   | 
| `options comments-dash`  | single line  | `--`  | 
| `options comments-slash`  | single line  | `//`  | 

Table: Comment options. {#tbl:id}

## Example

```rexx <!--comments.rexx-->
options levelb comments_dash comments_slash
/* rexx - slash star start slash 
   multiline comments always work  */
# a hash comment (single line also always works
import rxfnsb
/** 
 * this is a so-called doc comment
 * which has a special role in documenting code
 */
say 'comment on comments'
// this comment is enabled by option comments_clash
-- this comment is enabled by comments_dash
say 'no further comment.'
```
