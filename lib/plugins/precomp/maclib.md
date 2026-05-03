# REXX Macro Documentation

This document provides descriptions and usage examples for custom REXX macros.

| Macro                       | Description | Example |
|-----------------------------|-------------|---------|
| `repeatStr(s,n)`            | Repeats the given string s, n times. | `repeatStr("ha", 3)  -> "hahaha"` |
| `reverseStr(s)`             | Reverses the characters of the string. | `reverseStr("abc")  -> "cba"` |
| `ltrim(s)`                  | Removes leading whitespace. | `stripLeft("  hello")  -> "hello"` |
| `rtrim(s)`                  | Removes trailing whitespace. | `stripRight("hello  ")  -> "hello"` |
| `sign(n)`                   | Returns 1 if positive, -1 if negative, 0 if zero. | `sign(-5)  -> -1` |
| `gcd(a,b)`                  | Computes the greatest common divisor. | `gcd(12, 8)  -> 4` |
| `lcm(a,b)`                  | Computes the least common multiple. | `lcm(4, 6)  -> 12` |
| `ifNull(val,fallback)`      | Returns val if not empty, otherwise fallback. | `ifNull("", "default")  -> "default"` |
| `xor(a,b)`                  | Logical exclusive OR: true if only one is true. | `xor(1, 0)  -> 1` |
| `traceValue(v)`             | Prints the value with an arrow indicator. | `traceValue("X")  -> → X` |
| `error(msg)`                | Prints an error message and exits. | `error("Invalid input")  -> ERROR: Invalid input` |
| `warn(msg)`                 | Prints a warning message. | `warn("Deprecated call")  -> WARNING: Deprecated call` | 
| `startswith(suffix,string)` | Returns true if the string starts with the given prefix. | `startswith("Re", "Rexx")  -> 1` |
| `endswith(suffix,string)`   | Returns true if the string ends with the given suffix. | `endswith("xx", "Rexx")  -> 1` |
| `isDigit(string)`           | Returns true if the string contains only digits. | `isDigit("12345")  -> 1` |
| `isAlpha(string)`           | Returns true if the string contains only alphabetic letters. | `isAlpha("hello")  -> 1` |
| `isBlank(str)`              | Returns true if the string contains only space characters. | `isBlank("   ")  -> 1` |
| `isAlnum(str)`              | Returns true if the string contains only letters and digits. | `isAlnum("abc123")  -> 1` |
| `isSpace(str)`              | Returns true if the string consists only of spaces. | `isSpace("   ")  -> 1` |
| `isEmpty(str)`              | Returns true if the string is empty. | `isEmpty("")  -> 1` |
| `notEmpty(str)`             | Returns true if the string is not empty. | `notEmpty("x")  -> 1` |
| `isPunct(str)`              | Returns true if the string contains only punctuation characters. | `isPunct("!?")  -> 1` |
| `charAt(str, pos)`          | Returns the character at the given position. | `charAt("REXX", 2)  -> "E"` |
| `isLonger(str, len)`        | Returns true if the string is longer than the given length. | `isLonger("hello", 3)  -> 1` |
| `isShorter(str, len)`       | Returns true if the string is shorter than the given length. | `isShorter("hi", 5)  -> 1` |
| `equals(a, b)`              | Returns true if both values are equal (case-sensitive). | `equals("abc", "abc")  -> 1` |
| `equalsFold(a,b)`           | Returns true if both values are equal (case-insensitive). | `equalsFold("abc", "ABC")  -> 1` |
| `contains(str, sub)`        | Returns true if sub is found within str. | `contains("hello world", "world")  -> 1` |
| `isEven(n)`                 | Returns true if the number is even. | `isEven(4)  -> 1` |
| `isOdd(n)`                  | Returns true if the number is odd. | `isOdd(3)  -> 1` |
| `isPositive(n)`             | Returns true if the number is greater than zero. | `isPositive(5)  -> 1` |
| `isNegative(n)`             | Returns true if the number is less than zero. | `isNegative(-2)  -> 1` |
| `isZero(n)`                 | Returns true if the number is exactly zero. | `isZero(0)  -> 1` |
| `clamp(val,lo,hi)`          | Restricts val to the range [lo, hi]. | `clamp(15, 0, 10)  -> 10` |
| `between(n,a,b)`            | Returns true if n lies inclusively between a and b. | `between(5, 1, 10)  -> 1` |
| `inRange(n,a,b)`            | Alias for between: checks if n is in [a, b]. | `inRange(7, 5, 8)  -> 1` |
| `isLeapYear(y)`             | Returns true if the year is a leap year. | `isLeapYear(2024)  -> 1` |
