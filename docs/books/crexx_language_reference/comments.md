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

Multiline ('box') comments can be simple, or of the *flower pot* variety:

```bash
/*************************************
 *  This is a flower pot comment.
 *  Every line starts with '*'.
 *  Often used with headers.
 *************************************/
```

Also often seen is the lighter style where the asterisks are replaced by dashes in lines:

\newpage

```bash
/*-----------------------------------*
 *  Module: parser.crexx             *
 *  Purpose: expression parsing      *
 *-----------------------------------*/
```

or even

```bash
/*************************************************************\
*                                                            *
*            C R E X X   C O M P I L E R                     *
*                                                            *
\*************************************************************/
```


## Doc (Documentation) Comments

*Doc comments* are a special form of multiline comment which starts with `/**` and ends in `*/`. It is not processed by the \crexx{} compiler but can aid in the producing of documentation by external tools. In doc commments special tags as `@parm`, `@result` and `@author` can be used, purely for documentation purposes and with no consequence for the behaviour of the program.

The doc comment style used in the \crexx{} class library is:

```bash
      /**
	   * method insert
       * Inserts an element at the specified index.
       *
       * Elements at, and after the index are shifted to the right.
       *
       * @param index  1-based index where the element will be inserted.
       * @param item   The element to insert.
       */
```

for readability purposes.



## Single line comments

For \crexx{} level B, the *octothorpe* (#), also know as poundsign or hashmark, is the default for single line comments. Other single line comment styles can be chosen by the ```option``` statements ```comments_dash``` and ```comments_slash``` which enable the SQL, \nr{} and Object \rexx{} ```--``` double dash style, and ```comments_slash``` which enable the C++ style ```//``` line comments.

### Shebang

The # comment convention is a natural fit for the Unix, Linux and macOS convention where the first line of a program (or shell script) indicates the language processor to be used. This mechanism (sometimes called *shebang*, after the earlier *hash-bang*, which received its name after the colloquial names for the characters '#' (*hash*) and the '!' (*bang*) has its roots in the way an executable file is loaded on these operating systems. To execute a script `hello.crexx` (which needs to be made executable with the command `chmod +x hello.crexx`) we need to make sure that the *shebang* on its first line resembles this:

```rexx <!--helloshebang.rexx-->
#!/usr/bin/env crexx
say 'hello shebang!'
```

It can then be invoked with `./hello.crexx` from the current directory.

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
