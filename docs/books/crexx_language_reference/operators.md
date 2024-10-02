# Operators

## Expression Operators {#expression-operators}

Level B incorporates the Classic REXX operators. These operators function similarly to their counterparts in other programming languages. However, it's worth mentioning that Classic REXX, being untyped, allows for flexible use of these operators with different data types, which might lead to implicit conversions or unexpected behaviours. In Level B, this flexibility is supported by implicit promotions which may lead to errors at compile or runtime. 

## **Arithmetic operators** {#arithmetic-operators}

* \+ Add  
* \- Subtract  
* \* Multiply  
* / Divide  
* % Divide and return only the integer part of the quotient  
* // Divide and return only the remainder (not modulo because the result can be negative)  
* \*\* Raise the number to a whole-number power (exponentiation)  
* Prefix \- Negate the next term  
* Prefix \+ Take the next term as-is

## **Comparative operators** {#comparative-operators}

* \== Exactly equal (identical)  
* \= Equal (numerically or when padded)  
* ¬==, /== Not exactly equal (inverse of \==)  
* ¬=, /= Not equal (inverse of \=)  
* \> Greater than  
* \< Less than  
* \< \> Not equal  
* \>= Greater than or equal  
* ¬\< Not less than  
* \<= Less than or equal  
* ¬\> Not greater than

## **String Concatenation** {#string-concatenation}

* || Concatenate terms (you can use no blanks or one blank)  
* {space} Concatenate with a space added between terms  
* {abuttal} (i.e. no space between a literal and variable) Concatenate without a space added between terms

## **Logical operators** {#logical-operators}

* & AND (returns 1 if both terms are true)  
* | Inclusive OR (returns 1 if either term is true)  
* && Exclusive OR (returns 1 if either term is true, but not both)  
* Prefix ¬ Logical NOT (negates; 1 becomes 0 and vice-versa)

## **Term Operators** {#term-operators}

* () Parenthetical grouping  
* \[\] or . Array index

## **Operator priorities** {#operator-priorities}

The order of priority of the operators (from highest to lowest) is:

1. () \[\] . Term operators  
2. \+ \- ¬ Prefix operators  
3. \*\* Exponentiation  
4. \* / % // Multiply and divide  
5. \+ \- Add and subtract  
6. || Concatenation (with or without blank)  
7. \=, \>, … All comparison operators  
8. & And  
9. |, && Or, exclusive or
