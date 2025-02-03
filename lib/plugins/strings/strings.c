//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <string.h>

// New words function to count words
PROCEDURE(words) {
    char *wordstring = GETSTRING(ARG0); // Get the input string
    char *delim = GETSTRING(ARG1);       // Get the delimiter
    int word_count = 0;                  // Counter for the number of words

    // Check for NULL input
    if (wordstring == NULL) {
        RETURNINT(0); // Return 0 if the input string is NULL
    }

    // If delim is NULL or empty, set it to a space
    if (delim == NULL || strlen(delim) == 0) {
        delim = " "; // Default delimiter
    }

    // Tokenize the string and count words
    char *token = strtok(wordstring, delim); // Tokenize the string
    while (token != NULL) {
        word_count++; // Increment the word count
        token = strtok(NULL, delim); // Get the next token
    }

    RETURNINT(word_count); // Return the total word count
ENDPROC;
}

// New wordx function to retrieve subwords
PROCEDURE(word) {
    char *wordstring = GETSTRING(ARG0); // Get the input string
    int indx = GETINT(ARG1);             // Get the index of the desired subword
    char *delim = GETSTRING(ARG2);       // Get the delimiter
    int curword = 1;                     // Counter for the current word

    // Check for NULL input or invalid index
    if (wordstring == NULL || indx < 1) {
        RETURNSTRX(""); // Return empty string on error
    }
    // If delim is NULL or empty, set it to a space
    if (delim == NULL || strlen(delim) == 0) {
        delim = " "; // Default delimiter
    }

    // Create a temporary buffer for tokenization
    char *token = strtok(wordstring, delim); // Tokenize the string
    while (token != NULL) {
        if (curword == indx) {
            RETURNSTRX(token); // Return the found subword
        }
        token = strtok(NULL, delim); // Get the next token
        curword++;
    }

    RETURNSTRX(""); // Return empty string if index is out of bounds
ENDPROC;
}

// New lastword function to retrieve the last word
PROCEDURE(lastword) {
    char *wordstring = GETSTRING(ARG0); // Get the input string
    char *delim = GETSTRING(ARG1);       // Get the delimiter
    char *last_token = NULL;              // Variable to hold the last token

    // Check for NULL input
    if (wordstring == NULL) {
        RETURNSTRX(""); // Return empty string if the input string is NULL
    }

    // If delim is NULL or empty, set it to a space
    if (delim == NULL || strlen(delim) == 0) {
        delim = " "; // Default delimiter
    }

    // Tokenize the string and find the last word
    char *token = strtok(wordstring, delim); // Tokenize the string
    while (token != NULL) {
        last_token = token; // Update last_token with the current token
        token = strtok(NULL, delim); // Get the next token
    }

    RETURNSTRX(last_token ? last_token : ""); // Return the last token or empty string if none found
ENDPROC;
}

// New wordindex function to return the position of the requested word
PROCEDURE(wordindex) {
    char *wordstring = GETSTRING(ARG0); // Get the input string
    int indx = GETINT(ARG1);             // Get the index of the desired word
    char *delim = GETSTRING(ARG2);       // Get the delimiter
    int curword = 1;                     // Counter for the current word
    int position = 1;                    // Position of the current word

    // Check for NULL input or invalid index
    if (wordstring == NULL || indx < 1) {
        RETURNINT(-1); // Return -1 on error
    }

    // If delim is NULL or empty, set it to a space
    if (delim == NULL || strlen(delim) == 0) {
        delim = " "; // Default delimiter
    }

    // Tokenize the string and find the requested word's position
    char *token = strtok(wordstring, delim); // Tokenize the string
    while (token != NULL) {
        if (curword == indx) {
            RETURNINTX(position); // Return the position of the requested word
        }
        position += strlen(token) + strlen(delim); // Update position
        token = strtok(NULL, delim); // Get the next token
        curword++;
    }

    RETURNINT(0); // Return -1 if the word is not found
ENDPROC;
}

// New pos function to determine the first occurrence of a substring with an offset
PROCEDURE(pos) {
    char *substring = GETSTRING(ARG0);   // Get the substring to find
    char *wordstring = GETSTRING(ARG1);  // Get the input string
    int offset = GETINT(ARG2) - 1;         // Get the offset to start searching

    // Check for NULL input or invalid offset
    if (wordstring == NULL || substring == NULL || offset < 0) {
        RETURNINT(-1); // Return -1 on error
    }
    if(offset<0) offset=0;
    // Adjust the starting point for the search
    char *found = strstr(wordstring + offset, substring); // Find the first occurrence of the substring
    if (found != NULL) {
       RETURNINTX(found - wordstring + 1); // Return the position (1-based index)
    }

    RETURNINT(0); // Return 0 if the substring is not found
ENDPROC;
}

// New substring function to extract a substring from a string
PROCEDURE(substring) {
    char *wordstring = GETSTRING(ARG0); // Get the input string
    int position = GETINT(ARG1);         // Get the starting position (1-based index)
    int length = GETINT(ARG2);           // Get the length of the substring

    // Check for NULL input or invalid position
    if (wordstring == NULL || position < 1) {
        RETURNSTRX(""); // Return empty string on error
    }

    // Adjust position to 0-based index
    position--; // Convert to 0-based index

    // Calculate the actual length to copy
    int str_length = strlen(wordstring);
    if (length <= 0 || position + length > str_length) {
        length = str_length - position; // Take remaining string if length is <= 0
    }

    // Create a buffer for the substring
    char *result = (char *) malloc(length + 1); // +1 for null terminator
    if (result == NULL) {
        RETURNSTRX(""); // Return empty string on memory allocation failure
    }

    strncpy(result, wordstring + position, length); // Copy the substring
    result[length] = '\0'; // Null-terminate the substring

    RETURNSTRX(result); // Return the substring
    ENDPROC;
}

// New STRIP function to remove leading/trailing characters
PROCEDURE(strip) {
    char *wordstring = GETSTRING(ARG0); // Get the input string
    char *option = GETSTRING(ARG1);      // Get the option (B, L, T)
    char *char_to_strip = GETSTRING(ARG2); // Get the character to strip

    // Check for NULL input
    if (wordstring == NULL) {
        RETURNSTRX(""); // Return empty string on error
    }

    // Default to space if char_to_strip is empty
    if (char_to_strip == NULL || strlen(char_to_strip) == 0) {
        char_to_strip = " "; // Default character to strip
    }

    // Determine the length of the input string
    int str_length = strlen(wordstring);
    int start = 0;
    int end = str_length - 1;

    // Strip from the left
    if (option == NULL || option[0]=='L'|| option[0]=='B') {
        while (start < str_length && wordstring[start] == char_to_strip[0]) {
            start++;
        }
    }

    // Strip from the right
    if (option == NULL || option[0]=='T'|| option[0]=='B') {
        while (end >= start && wordstring[end] == char_to_strip[0]) {
            end--;
        }
    }

    // Create a new string for the result
    int new_length = end - start + 1;
    char *result = (char *)malloc(new_length + 1); // +1 for null terminator
    if (result == NULL) {
        RETURNSTRX(""); // Return empty string on memory allocation failure
    }

    // Copy the stripped string
    strncpy(result, wordstring + start, new_length);
    result[new_length] = '\0'; // Null-terminate the result

    RETURNSTRX(result); // Return the stripped string
    ENDPROC;
}

// New DELSTR function to delete a part of a string
PROCEDURE(delstr) {
    char *wordstring = GETSTRING(ARG0); // Get the input string
    int start = GETINT(ARG1);            // Get the starting position (1-based index)
    int length = GETINT(ARG2);           // Get the length of the substring to delete

    // Check for NULL input or invalid start position
    if (wordstring == NULL || start < 1) {
        RETURNSTRX(""); // Return empty string on error
    }

    // Adjust start to 0-based index
    start--; // Convert to 0-based index

    // Calculate the actual length of the string
    int str_length = strlen(wordstring);
    if (length <= 0 || start + length > str_length) {
        length = str_length - start; // Take remaining string if length is <= 0
    }

    // Create a buffer for the new string
    char *result = (char *)malloc(str_length - length + 1); // +1 for null terminator
    if (result == NULL) {
        RETURNSTRX(""); // Return empty string on memory allocation failure
    }

    // Copy the part before the deletion
    strncpy(result, wordstring, start);
    result[start] = '\0'; // Null-terminate the first part

    // Append the part after the deletion
    strcat(result, wordstring + start + length); // Concatenate the remaining part

    RETURNSTRX(result); // Return the modified string
    ENDPROC;
}

// New ABBREV function to check if target is an abbreviation of source
PROCEDURE(ABBREV) {
    char *target = GETSTRING(ARG0); // Get the target string
    char *source = GETSTRING(ARG1); // Get the source string
    int min = GETINT(ARG2);          // Get the minimum number of characters

    // Check for NULL input
    if (target == NULL || source == NULL) {
        RETURNINT(0); // Return 0 on error
    }

    // Check if target is longer than source or if min is greater than the length of target
    if (strlen(target) > strlen(source) || min > strlen(target)) {
        RETURNINT(0); // Not an abbreviation
    }

    // Compare the target with the beginning of the source
    if (strncmp(target, source, strlen(target)) == 0 && strlen(target) >= min) {
        RETURNINTX(1); // It's an abbreviation
    }

    RETURNINT(0); // Not an abbreviation
ENDPROC
}

// New WORDPOS function to find the position of a word in a phrase
PROCEDURE(WORDPOS) {
    char *searchWord = GETSTRING(ARG0); // Get the word to search for
    char *phrase = GETSTRING(ARG1);      // Get the phrase to search in
    char *delim = GETSTRING(ARG2);       // Get the delimiter

    int position = 1;                    // Position counter (1-based index)

    // Check for NULL input
    if (searchWord == NULL || phrase == NULL) {
        RETURNINT(0); // Return 0 if either input is NULL
    }
    if (delim == NULL || strlen(delim) == 0) {
        delim = " "; // Default delimiter
    }

    // Tokenize the phrase and search for the word
    char *token = strtok(phrase, delim); // Tokenize using space as delimiter
    while (token != NULL) {
        if (strcmp(token, searchWord) == 0) {
            RETURNINT(position); // Return the position if found
        }
        token = strtok(NULL, delim); // Get the next token
        position++; // Increment position counter
    }

    RETURNINT(0); // Return 0 if the word is not found
}

// New SUBWORD function to extract a specified number of words from a phrase
PROCEDURE(SUBWORD) {
    char *phrase = GETSTRING(ARG0); // Get the input phrase
    int position = GETINT(ARG1);     // Get the position of the first word (1-based index)
    int numberOfWords = GETINT(ARG2); // Get the number of words to extract
    char *delim = GETSTRING(ARG3);       // Get the delimiter
    int curword = 1;                 // Counter for the current word
    int plen=strlen(phrase);
    char *result = (char *)malloc(plen); // Buffer for the result (adjust size as needed)
    int resultLength = 0;            // Length of the result

 // Check for NULL input
    if (phrase == NULL || position < 1) {
        RETURNSTRX(""); // Return empty string if input is invalid
    }
    if (delim == NULL || strlen(delim) == 0) {
        delim = " "; // Default delimiter
    }
    // Default to extract all remaining words if numberOfWords is not specified
    if (numberOfWords <= 0) {
        numberOfWords = INT_MAX; // Set to a large number to extract all remaining words
    }

    // Tokenize the phrase and extract the specified words
    char *token = strtok(phrase, delim); // Tokenize using space as delimiter
    while (token != NULL) {
        if (curword >= position && resultLength < plen) { // Check if within range and buffer limit
            if (resultLength > 0) {
                strcat(result, " "); // Add space before the next word
                resultLength++;
            }
            strcat(result, token); // Append the word to the result
            resultLength += strlen(token);
            if (--numberOfWords == 0) {
                break; // Stop if the required number of words is extracted
            }
        }
        token = strtok(NULL, delim); // Get the next token
        curword++; // Increment word counter
    }

    RETURNSTRX(result); // Return the extracted words
    ENDPROC
}

// New WORDLEN function to retrieve the length of the n-th word
PROCEDURE(WORDLEN) {
    char *string = GETSTRING(ARG0);  // Get the input string
    int wordNumber = GETINT(ARG1);   // Get the word number (1-based index)
    char *delim = GETSTRING(ARG2);   // Get the delimiter

    int curword = 1;                 // Counter for the current word

    // Check for NULL input
    if (string == NULL || wordNumber < 1) {
        RETURNINT(0); // Return 0 if input is invalid
    }
    if (delim == NULL || strlen(delim) == 0) {
        delim = " "; // Default delimiter
    }

    // Tokenize the string and find the specified word
    char *token = strtok(string, delim); // Tokenize using space as delimiter
    while (token != NULL) {
        if (curword == wordNumber) {
           RETURNINTX(strlen(token)); // Return the length of the specified word
        }
        token = strtok(NULL, delim); // Get the next token
        curword++; // Increment word counter
    }

    RETURNINT(0); // Return 0 if the word number is out of range
ENDPROC
}

// Linked List definitions
LOADFUNCS
   ADDPROC(words, "strings.xwords", "b",  ".int","string = .string,delim=.string");
   ADDPROC(word, "strings.xword", "b",  ".string","string = .string,indx=.int,delim=.string");
   ADDPROC(lastword, "strings.xlastword", "b",  ".string","string = .string,delim=.string");
   ADDPROC(wordindex, "strings.xwordindex", "b",  ".int","string = .string,indx=.int,delim=.string");
   ADDPROC(pos, "strings.xpos", "i",  ".int","string = .string,substring=.string,offset=.int");
   ADDPROC(substring, "strings.xsubstr", "b",  ".string","string = .string,position=.int,length=.int");
   ADDPROC(delstr, "strings.xdelstr", "b",  ".string","string = .string,start=.int,length=.int");
   ADDPROC(strip, "strings.xstrip", "b",  ".string","string = .string,option=.string,char=.string");
   ADDPROC(ABBREV, "strings.xabbrev", "i",  ".int","target=.string,source=.string,min=.int");
   ADDPROC(WORDPOS, "strings.xwordpos", "i",  ".int","searchWord=.string,phrase=.string");
   ADDPROC(SUBWORD, "strings.xsubword", "b",  ".string","string = .string,position=.int,numberOfWords=.int,delim=.string");
   ADDPROC(WORDLEN, "strings.xwordlen", "i",  ".int","string = .string,wordNumber=.int,delim=.string");
ENDLOADFUNCS