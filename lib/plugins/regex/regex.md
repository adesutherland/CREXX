# Regex Plugin Documentation

## Overview
The regex plugin provides regular expression functionality and string distance calculations for CREXX/PA. It includes pattern matching, compilation, and string distance metrics.

## Functions

### RXCOMPILE(pattern, flags)
Compiles a regular expression pattern.

Parameters:
- pattern: String containing the regular expression
- flags: Integer containing compilation flags
  - RX_BASIC (0): Basic Regular Expressions (BRE)
  - RX_EXTENDED (1): Extended Regular Expressions (ERE)
  - RX_ICASE (2): Case insensitive matching
  - RX_NEWLINE (4): Honor newline as special character
  - RX_NOSUB (8): Only report success/failure

Returns:
- handle: Positive integer handle on success
- Negative error code on failure:
  - RX_ERROR_PARAM (-1): Invalid parameters
  - RX_ERROR_MEMORY (-2): Memory allocation failed
  - RX_ERROR_COMPILE (-3): Pattern compilation failed

### RXMATCH(handle, string, flags)
Matches a compiled pattern against a string.

Parameters:
- handle: Handle returned by rxcompile
- string: String to match against
- flags: Integer containing match flags
  - RX_NOTBOL (16): ^ doesn't match beginning of string
  - RX_NOTEOL (32): $ doesn't match end of string

Returns:
- 1: Match found
- 0: No match
- Negative error code on failure

### RXFREE(handle)
Frees resources associated with a compiled pattern.

Parameters:
- handle: Handle returned by rxcompile

Returns:
- RX_SUCCESS (0): Operation successful
- RX_ERROR_PARAM (-1): Invalid handle

### RXERROR(handle)
Returns error message for last operation on handle.

Parameters:
- handle: Handle returned by rxcompile

Returns:
- String containing error message
- "Invalid handle" if handle is invalid
- "No error" if no error occurred

### LEVENSHTEIN(string1, string2)
Calculates Levenshtein (edit) distance between two strings.

Parameters:
- string1: First string
- string2: Second string

Returns:
- Positive integer representing edit distance
- -1: Invalid parameters
- -2: Memory allocation error

### HAMMING(string1, string2, uppercase)
Calculates Hamming distance between two equal-length strings.

Parameters:
- string1: First string
- string2: Second string
- uppercase: Convert to uppercase before comparison (1=yes, 0=no)

Returns:
- Positive integer representing Hamming distance
- -1: Invalid parameters
- -2: Strings must be equal length


## Example Usage
### Regular Expression Examples

```rexx
/* Define common flags */
RX_BASIC    = 0      /* Basic Regular Expressions (BRE) */
RX_EXTENDED = 1      /* Extended Regular Expressions (ERE) */
RX_ICASE    = 2      /* Case insensitive matching */
RX_NEWLINE  = 4      /* Honor newline as special character */
RX_NOSUB    = 8      /* Only report success/failure */

/* Basic alphanumeric validation */
pattern = '^[A-Za-z0-9]+$'
handle = rxcompile(pattern, RX_EXTENDED)
if rxmatch(handle, "Test123", 0) then say "Match!"
call rxfree handle

/* Email validation */
pattern = '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$'
handle = rxcompile(pattern, RX_EXTENDED)
if rxmatch(handle, "test@example.com", 0) then say "Valid email"
call rxfree handle

/* Case insensitive matching */
pattern = 'hello world'
handle = rxcompile(pattern, RX_EXTENDED + RX_ICASE)
if rxmatch(handle, "HELLO WORLD", 0) then say "Case insensitive match!"
call rxfree handle

/* Date validation */
pattern = '^(19|20)\d\d[- /.](0[1-9]|1[012])[- /.](0[1-9]|[12][0-9]|3[01])$'
handle = rxcompile(pattern, RX_EXTENDED)
if rxmatch(handle, "2023-12-31", 0) then say "Valid date"
call rxfree handle
```

### String Distance Examples

```rexx
/* Levenshtein distance examples */
say levenshtein("kitten", "sitting")    /* Returns: 3 */
say levenshtein("hello", "hallo")       /* Returns: 1 */
say levenshtein("book", "cook")         /* Returns: 1 */

/* Hamming distance examples */
say hamming("hello", "HELLO", 1)        /* Returns: 0 (case insensitive) */
say hamming("book", "cook", 0)          /* Returns: 1 (case sensitive) */
say hamming("test", "best", 0)          /* Returns: 1 */
```

### Error Handling Example

```rexx
/* Testing invalid patterns */
handle = rxcompile("[invalid", RX_EXTENDED)
if handle < 0 then do
    say rxerror(handle)    /* Shows compilation error */
end
```

## Error Handling
All functions return error codes as negative integers. Use rxerror() to get detailed error messages:
rexx
handle = rxcompile("[invalid", 0)
if handle < 0 then
say rxerror(handle)

## Notes
- The plugin uses the system regex library (regex.h)
- Memory is automatically freed when calling rxfree()
- Hamming distance requires strings of equal length
- Levenshtein distance works with strings of any length

## Error Codes
The following error codes are used throughout the plugin:

| Code | Constant | Description |
|------|----------|-------------|
| 0 | RX_SUCCESS | Operation completed successfully |
| -1 | RX_ERROR_PARAM | Invalid parameters provided |
| -2 | RX_ERROR_MEMORY | Memory allocation failed |
| -3 | RX_ERROR_COMPILE | Pattern compilation failed |
| -4 | RX_ERROR_EXEC | Pattern matching execution failed |

### Function-Specific Error Codes

#### Levenshtein Distance
- -1: Invalid parameters (NULL strings)
- -2: Memory allocation error

#### Hamming Distance
- -1: Invalid parameters (NULL strings)
- -2: Strings must be equal length

