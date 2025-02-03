# Strings Module Documentation

This module provides various string manipulation functions, including word counting, retrieving specific words, and finding the position of words within a string.
All these functions are also available as standard REXX functions. However, to distinguish them, the additional functions in this module are prefixed with an "X". The key enhancement is the inclusion of an additional delimiter parameter for word splitting, which applies to all word-related functions.
Another advantage of these functions is improved performance. Since they run in plain C mode, they avoid the overhead of multiple REXX assembler statements.
These functions work exclusively with Single-Byte Character Sets (SBCS), such as ASCII or EBCDIC. Multi-Byte Character Sets (MBCS), including UTF-8 and Unicode, are not reliably supported.
## Functions

### 1. `XWORDS(string, delim)`

**Description:**
Counts the number of words in a given string using a specified delimiter for separation (default is a space).

**Parameters:**
- `string`: The input string to be processed.
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- An integer representing the total number of words in the string.

**Example:**
```c
count = XWORDS("Hello world! This is a test.", " "); // count = 6
```

---

### 2. `XWORD(string, indx, delim)`

**Description:**
Retrieves the word at a specified index from a given string using a specified delimiter for separation (default is a space).

**Parameters:**
- `string`: The input string to be processed.
- `indx`: The index of the desired word (1-based index).
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- The word at the specified index, or an empty string if the index is out of bounds.

**Example:**
```c
secondWord = XWORD("Hello world! This is a test.", 2, " "); // secondWord = "world!"
```

---

### 3. `XLASTWORD(string, delim)`

**Description:**
Retrieves the last word from a given string based on a specified delimiter for separation (default is a space).

**Parameters:**
- `string`: The input string to be processed.
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- The last word in the string, or an empty string if no words are found.

**Example:**
```c
lastWord = XLASTWORD("Hello world! This is a test.", " "); // lastWord = "test."
```

---

### 4. `XWORDINDEX(string, indx, delim)`

**Description:**
Returns the starting position of a specified word in a given string using a specified delimiter for separation (default is a space).

**Parameters:**
- `string`: The input string to be processed.
- `indx`: The index of the desired word (1-based index).
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- An integer representing the starting position of the specified word in the string, or 0 if the word is not found.

**Example:**
```c
position = XWORDINDEX("Hello world! This is a test.", 3, " "); // position = 14
```

---
### 5. `XWORDPOS(searchWord, phrase, delim)`

**Description:**
Finds the position of a specified word within a given phrase.

**Parameters:**
- `searchWord`: The word you want to find.
- `phrase`: The string where you are searching.
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- Returns the word position (1-based index) if found.
- Returns 0 if the word is not found.

**Example:**
```c
position = XWORDPOS("world", "Hello world! This is a test.", " "); // position = 2
```

---

### 6. `XSUBWORD(phrase, position, numberOfWords, delim)`

**Description:**
Extracts a specified number of words from a given phrase based on a specified position.

**Parameters:**
- `phrase`: The string from which you want to extract words.
- `position`: The position of the first word in the string (1-based index).
- `numberOfWords`: The number of words to extract. If not specified or less than or equal to 0, defaults to all remaining words.
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- Returns the extracted words as a single string.
- If the position is out of range, it returns an empty string.

**Example:**
```c
extractedWords = XSUBWORD("Hello world! This is a test.", 3, 2, " "); // extractedWords = "This is"
```

---

### 7. `XWORDLEN(string, word-number, delim)`

**Description:**
Retrieves the length of the n-th word from a given string.

**Parameters:**
- `string`: The input string from which to retrieve the word.
- `word-number`: The position of the word whose length you want to find (1-based index).
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- Returns the length of the specified word if found.
- Returns 0 if the word number is out of range.

**Example:**
```c
length = XWORDLEN("Hello world! This is a test.", 3, " "); // length = 4
```

---

### 8. `XPOS(substring, wordstring, offset, delim)`

**Description:**
Determines the first occurrence of a substring within a string, starting from a specified offset.

**Parameters:**
- `substring`: The substring to find.
- `wordstring`: The input string to search in.
- `offset`: The position to start searching from (1-based index).
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- The position (1-based index) of the first occurrence of the substring if found, or -1 if not found.

**Example:**
```c
foundPosition = XPOS("world", "Hello world! This is a test.", 1, " "); // foundPosition = 7
```

---

### 9. `XSUBSTR(wordstring, position, length, delim)`

**Description:**
Extracts a substring from a string based on a specified starting position and length.

**Parameters:**
- `wordstring`: The input string from which to extract the substring.
- `position`: The starting position (1-based index).
- `length`: The length of the substring to extract.
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- The extracted substring, or an empty string if the input is invalid.

**Example:**
```c
substring = XSUBSTR("Hello world! This is a test.", 1, 5, " "); // substring = "Hello"
```

---

### 10. `XSTRIP(string, option, char, delim)`

**Description:**
Removes leading and/or trailing characters from a given string based on specified options.

**Parameters:**
- `string`: The input string to be processed.
- `option`: A single character that specifies which side(s) to strip:
  - `B`: Both leading and trailing characters.
  - `L`: Leading characters only.
  - `T`: Trailing characters only.
- `char`: The character to remove. If empty, defaults to a space.
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- The modified string after stripping, or an empty string if the input is invalid.

**Example:**
```c
stripped = XSTRIP("   Hello world!   ", "B", " ", " "); // stripped = "Hello world!"
```

---

### 11. `XABBREV(target, source, min, delim)`

**Description:**
Checks if the target string is an abbreviation of the source string.

**Parameters:**
- `target`: The target string to check.
- `source`: The source string to compare against.
- `min`: The minimum number of characters that need to be equal.
- `delim`: The delimiter used to separate words (optional). If not provided or empty, defaults to a space (" ").

**Returns:**
- Returns 1 if the target is an abbreviation of the source, or 0 otherwise.

**Example:**
```c
isAbbrev = XABBREV("Hel", "Hello", 3, " "); // isAbbrev = 1
```

---


## Usage Example

```c
// Example usage of the functions
count = XWORDS("Hello world! This is a test.", " "); // count = 6
secondWord = XWORD("Hello world! This is a test.", 2, " "); // secondWord = "world!"
lastWord = XLASTWORD("Hello world! This is a test.", " "); // lastWord = "test."
position = XWORDINDEX("Hello world! This is a test.", 3, " "); // position = 14
```

## Notes
- All functions are case-sensitive and treat different cases as distinct characters.
- Ensure that the input strings are properly formatted to avoid unexpected results.
