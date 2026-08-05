# Parsing templates

The `parse` instruction allows a selected string to be parsed (split up) and
assigned to variables, under the control of a *template*.

The various mechanisms in the template allow a string to be split up by explicit
matching of strings (called *patterns*), or by specifying numeric positions
(*positional patterns* - for example, to extract data from particular columns of
a line read from a character stream). Once split into parts, each segment of the
string can then be assigned to variables as a whole or by words (delimited by
blanks).

This section first gives some informal examples of how the parsing template can
be used, and then defines the algorithms in detail.

## Introduction to parsing

The simplest form of parsing template consists of a list of variable names. The
string being parsed is split up into words (characters delimited by blanks), and
each word from the string is assigned to a variable in sequence from left to
right. The final variable is treated specially in that it will be assigned
whatever is left of the original string and may therefore contain several words.
For example, in the `parse` instruction:

```rexx <!--parse1.crexx-->
    parse 'This is a sentence.' v1 v2 v3
```

the term (in this case a literal string) following the instruction keyword is
parsed, and then: the variable ***v1*** would be assigned the value "**This**",
***v2*** would be assigned the value "**is**", and ***v3*** would be assigned
the value "**a sentence.**".

Leading blanks are removed from each word in the string before it is assigned to
a variable, as is the blank that delimits the end of the word. Thus, variables
set in this manner (***v1*** and ***v2*** in the example) will never have
leading or trailing blanks, though ***v3*** could have both leading and trailing
blanks. Note that the variables assigned values in a template are always given a
new value and so if there are fewer words in the string than variables in the
template then the unused variables will be set to the null string. The second
parsing mechanism uses a literal string in a template as a pattern, to split up
the string. For example:

```rexx <!--parse2.crexx-->
    parse 'To be, or not to be?' w1 ',' w2
```

would cause the string to be scanned for the comma, and then split at that
point; the variable ***w1*** would be set to "**To be**", and ***w2*** is set to
" **or not to be?**". Note that the pattern itself (and **only** the pattern) is
removed from the string. Each section of the string is treated in just the same
way as the whole string was in the previous example, and so either section could
be split up into words. Thus, in:

```rexx <!--parse3.crexx-->
    parse 'To be, or not to be?' w1 ',' w2 w3 w4
```

***w2*** and ***w3*** would be assigned the values "**or**" and "**not**", and
***w4*** would be assigned the remainder: "**to be?**".

If the string in the last example did not contain a comma, then the pattern
would effectively "match" the end of the string, so the variable to the left of
the pattern would get the entire input string, and the variables to the right
would be set to a null string. The pattern may be specified as a variable, by
putting the variable name in parentheses. The following instructions therefore
have the same effect as the last example:

```rexx <!--parse4.crexx-->
    c=','
    parse 'To be, or not to be?' w1 (c) w2 w3 w4
```

The third parsing mechanism is the numeric positional pattern. This works in the
same way as the string pattern except that it specifies a column number. So:

```rexx <!--parse5.crexx-->
    parse 'Flying pigs have wings' x1 5 x2
```

would split the string at the fifth column, so ***x1*** would be "**Flyi**" and
***x2*** would start at column 5 and so be "**ng pigs have wings**". More than
one pattern is allowed, so for example:

```rexx <!--parse6.crexx-->
    parse 'Flying pigs have wings' x1 5 x2 10 x3
```

would split the string at columns 5 and 10, so ***x2*** would be "**ng pi**" and
***x3*** would be "**gs have wings**". The numbers can be relative to the last
number used, so:

```rexx <!--parse7.crexx-->
    parse 'Flying pigs have wings' x1 5 x2 +5 x3
```

would have exactly the same effect as the last example; here the **+5** may be
thought of as specifying the length of the string to be assigned to ***x2***. As
with literal string patterns, the positional patterns can be specified as a
variable by putting the name of a variable, in parentheses, in place of the
number. An absolute column number should then be indicated by using an equals
sign ("**=**") instead of a plus or minus sign. The last example could therefore
be written:

```rexx <!--parse8.crexx-->
    start=5
    length=5
    data='Flying pigs have wings'
    parse data  x1 =(start) x2 +(length) x3
```

String patterns and positional patterns can be mixed (in effect the beginning of
a string pattern just specifies a variable column number) and some very powerful
things can be done with templates. The next section describes in more detail how
the various mechanisms interact.

## Parsing definition

This section describes the rules that govern parsing. In its most general form,
a template consists of alternating pattern specifications and variable names.
Blanks may be added between patterns and variable names to separate the tokens
and to improve readability. The patterns and variable names are used strictly in
sequence from left to right, and are used once only. In practice, various
simpler forms are used in which either variable names or patterns may be
omitted; we can therefore have variable names without patterns in between, and
patterns without intervening variable names. In general, the value assigned to a
variable is that sequence of characters in the input string between the point
that is matched by the pattern on its left and the point that is matched by the
pattern on its right. If the first item in a template is a variable, then there
is an implicit pattern on the left that matches the start of the string, and
similarly if the last item in a template is a variable then there is an implicit
pattern on the right that matches the end of the string. Hence the simplest
template consists of a single variable name which in this case is assigned the
entire input string. Setting a variable during parsing is identical in effect to
setting a variable in an assignment. The constructs that may appear as patterns
fall into two categories; patterns that act by searching for a matching string
(literal patterns), and numeric patterns that specify an absolute or relative
position in the string (positional patterns). Either of these can be specified
explicitly in the template, or alternatively by a reference to a variable whose
value is to be used as the pattern. For the following examples, assume that the
following sample string is being parsed; note that all blanks are significant -
there are two blanks after the first word "**is**" and also after the second
comma:

```rexx <!--parse9.crexx-->
    'This is  the text which, I think,  is scanned.'
```

### Parsing with literal patterns

Literal patterns cause scanning of the data string to find a sequence that
matches the value of the literal. Literals are expressed as a quoted string. The
null string matches the end of the data. The template:

```rexx <!--parse10.crexx-->
    w1 ',' w2 ',' w3
```

when parsing the sample string, results in:

```rexx <!--parse11.crexx-->
    w1 has the value "This is  the text which"
    w2 has the value " I think"
    w3 has the value "  is scanned."
```

Here the string is parsed using a template that asks that each of the variables
receive a value corresponding to a portion of the original string between
commas; the commas are given as quoted strings. Note that the patterns
themselves are removed from the data being parsed. A different parse would
result with the template:

```rexx <!--parse12.crexx-->
    w1 ',' w2 ',' w3 ',' w4
```

which would result in:

```rexx <!--parse13.crexx-->
    w1 has the value "This is  the text which"
    w2 has the value " I think"
    w3 has the value "  is scanned."
    w4 has the value ""  (null string)
```

This illustrates an important rule. When a match for a pattern cannot be found
in the input string, it instead "matches" the end of the string. Thus, no match
was found for the third **’,’** in the template, and so ***w3*** was assigned
the rest of the string. ***w4*** was assigned a null string because the pattern
on its left had already reached the end of the string. Note that **all**
variables that appear in a template in this way are assigned a new value.

### Parsing strings into words

If a variable is directly followed by one or more other variables, then the
string selected by the patterns is assigned to the variables in the following
manner. Each blank-delimited word in the string is assigned to each variable in
turn, except for the last variable in the group (which is assigned the remainder
of the string). The values of the variables which are assigned words will have
neither leading nor trailing blanks. Thus the template:

```rexx <!--parse14.crexx-->
    w1 w2 w3 w4 ','
```

would result in:

```rexx <!--parse15.crexx-->
    w1 has the value "This'
    w2 has the value "is"
    w3 has the value "the"
    w4 has the value "text which"
```

Note that the final variable (***w4*** in this example) could have had both
leading blanks and trailing blanks, since only the blank that delimits the
previous word is removed from the data. Also observe that this example is not
the same as specifying explicit blanks as patterns, as the template:

```rexx <!--parse16.crexx-->
    w1 ' ' w2 ' ' w3 ' ' w4 ','
```

would in fact result in:

```rexx <!--parse17.crexx-->
    w1 has the value "This'
    w2 has the value "is"
    w3 has the value ""  (null string)
    w4 has the value "the text which"
```

since the third pattern would match the third blank in the data. In general,
when a variable is followed by another variable then parsing of the input into
individual words is implied. The parsing process may be thought of as first
splitting the original string up into other strings using the various kinds of
patterns, and then assigning each of these new strings to (zero or more)
variables.

### Use of the period as a placeholder

A period (separated from any symbols by at least one blank) acts as a
placeholder in a template. It has exactly the same effect as a variable name,
except that no variable is set. It is especially useful as a "dummy variable" in
a list of variables, or to collect (ignore) unwanted information at the end of a
string. Thus the template:

```rexx <!--parse18.crexx-->
     . . . word4 .
```
would extract the fourth word ("**text**") from the sample string and place it
in the variable ***word4***. Blanks between successive periods in templates may
be omitted, so the template:

```rexx <!--parse19.crexx-->
     ... word4 .
```

would have the same result as the last template.

### Parsing with positional patterns

Positional patterns may be used to cause the parsing to occur on the basis of
position within the string, rather than on its contents. They take the form of
whole numbers, optionally preceded by a plus, minus, or equals sign which
indicate relative or absolute positioning. These may cause the matching
operation to "back up" to an earlier position in the data string, which can only
occur when positional patterns are used. **Absolute positional patterns:** A
number in a template that is **not** preceded by a sign refers to a particular
(absolute) character column in the input, with 1 referring to the first column.
For example, the template:

```rexx <!--parse20.crexx-->
    s1 10 s2 20 s3
```

results in:

```rexx <!--parse21.crexx-->
    s1 has the value "This is  "
    s2 has the value "the text w"
    s3 has the value "hich, I think,  is scanned."
```

Here ***s1*** is assigned characters from the first through the ninth character,
and ***s2*** receives input characters 10 through 19. As usual the final
variable, ***s3***, is assigned the remainder of the input.

An equals sign ("**=**") may be placed before the number to indicate explicitly
that it is to be used as an absolute column position; the last template could
have been written:

```rexx <!--parse22.crexx-->
    s1 =10 s2 =20 s3
```

A positional pattern that has no sign or is preceded by the equals sign is known
as an *absolute positional pattern*. **Relative positional patterns:** A number
in a template that is preceded by a plus or minus sign indicates movement
relative to the character position at which the previous pattern match occurred.
This is a *relative positional pattern*. If a plus or minus is specified, then
the position used for the next match is calculated by adding (or subtracting)
the number given to the last matched position. The last matched position is the
position of the first character of the last match, whether specified numerically
or by a string.

For example, the instructions:

```rexx <!--parse23.crexx-->
    parse '123456789'  3 w1 +3 w2 3 w3
```

result in

```rexx <!--parse24.crexx-->
    w1 has the value "345"
    w2 has the value "6789"
    w3 has the value "3456789"
```

The **+3** in this case is equivalent to the absolute number **6** in the same
position, and may also be considered to be specifying the length of the data
string to be assigned to the variable ***w1***. This example also illustrates
the effects of a positional pattern that implies movement to a character
position to the left of (or to) the point at which the last match occurred. The
variable on the left is assigned characters through the end of the input, and
the variable on the right is, as usual, assigned characters starting at the
position dictated by the pattern. A useful effect of this is that multiple
assignments can be made:

```rexx <!--parse25.crexx-->
    parse x 1 w1 1 w2 1 w3
```

This results in assigning the (entire) value of ***x*** to ***w1***, ***w2***,
and ***w3***. (The first "**1**" here could be omitted as it is effectively the
same as the implicit starting pattern described at the beginning of this
section.) If a positional pattern specifies a column that is greater than the
length of the data, it is equivalent to specifying the end of the data (*i.e.*,
no padding takes place). Similarly, if a pattern specifies a column to the left
of the first column of the data, this is not an error but instead is taken to
specify the first column of the data. Any pattern match sets the "last position"
in a string to which a relative positional pattern can refer. The "last
position" set by a literal pattern is the position at which the match occurred,
that is, the position in the data of the *first* character in the pattern. The
literal pattern in this case is **not** removed from the parsed data. Thus the
template:

```rexx <!--parse26.crexx-->
    ',' -1 x +1
```

will:

1.  Find the first comma in the input (or the end of the string if there is no
    comma).

2.  Back up one position.

3.  Assign one character (the character immediately preceding the comma or end
    of string) to the variable ***x***.

One possible application of this is looking for abbreviations in a string. Thus
the instruction:

```rexx <!--parse27.crexx-->
    /* Ensure options have a leading blank and are
       in uppercase before parsing. */
    parse (' 'opts).upper ' PR' +1 prword ' '
```

will set the variable ***prword*** to the first word in ***opts*** that starts
with "**PR**" (in any case), or will set it to the null string if no such word
exists. **Notes:**

1.  The positional patterns **+0** and **-0** are valid, have the same effect,
    and may be used to include the whole of a previous literal (or variable)
    pattern within the data string to be parsed into any following variables.

2.  As illustrated in the last example, patterns may follow each other in the
    template without intervening variable names. In this case each pattern is
    obeyed in turn from left to right, as usual.

3.  There may be blanks between the sign in a positional pattern and the number,
    because cREXX defines that blanks adjacent to special characters are
    removed.

### Parsing with variable patterns

It is sometimes desirable to be able to specify a pattern by using the value of
a variable instead of a fixed string or number. This may be achieved by placing
the name of the variable to be used as the pattern in parentheses (blanks are
not necessary either inside or outside the parentheses, but may be added if
desired). This is called a *variable reference*; the value of the variable is
converted to string before use, if necessary. If the parenthesis to the left of
the variable name is not preceded by an equals, plus, or minus sign ("**=**",
"**+**", or "**-**") the value of the variable is then used as though it were a
literal (string) pattern. The variable may be one that has been set earlier in
the parsing process, so for example:

```rexx <!--parse28.crexx-->
    input="L/look for/1 10"
    parse input  verb 2 delim +1 string (delim) rest
```

will set:

```rexx <!--parse29.crexx-->
    verb to 'L'
    delim to '/'
    string to 'look for'
    rest to '1 10'
```

If the left parenthesis **is** preceded by an equals, plus, or minus sign then
the value of the variable is used as an absolute or relative positional pattern
(instead of as a literal string pattern). In this case the value of the variable
must be a non-negative whole number, and (as before) it may have been set
earlier in the parsing process.
