#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <limits.h>   // Include this for INT_MAX
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"  // crexx/pa - Plugin Architecture header file

// Function prototypes
long long evaluate(const char **expr);
long long parse_expression(const char **expr);
int parse_binary_number(const char **expr);
// remove white spaces on both sides
char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++; // Skip leading spaces

    if (*str == '\0') return str; // If string is all spaces, return empty

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--; // Trim trailing spaces

    *(end + 1) = '\0'; // Null-terminate the trimmed string
    return str;
}

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

    RETURNSTR(result); // Return the substring
    free(result);
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
    char *source = GETSTRING(ARG0); // Get the source string
    char *target = GETSTRING(ARG1); // Get the target string

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
#define MAX_STACK_SIZE 100

// Function to parse a hexadecimal number
int parse_hex_number(const char **expr) {
    int num = 0;
    (*expr) += 2;  // Skip '0x'
    while (isxdigit(**expr)) {
        num = num * 16 + (**expr >= 'a' ? **expr - 'a' + 10 : 
                         **expr >= 'A' ? **expr - 'A' + 10 : 
                         **expr - '0');
        (*expr)++;
    }
    return num;
}

// Function to parse a decimal number
int parse_decimal_number(const char **expr) {
    int num = 0;
    while (isdigit(**expr)) {
        num = num * 10 + (**expr - '0');
        (*expr)++;
    }
    return num;
}

// Function to parse any number (hex, decimal, or binary)
int parse_number(const char **expr) {
    if (**expr == '0') {
        if (*(*expr + 1) == 'b' || *(*expr + 1) == 'B') {
            (*expr) += 2;  // Skip '0b' or '0B'
            return parse_binary_number(expr);  // Call a new function to parse binary
        } else if (*(*expr + 1) == 'x' || *(*expr + 1) == 'X') {
            return parse_hex_number(expr);  // Call hex parser
        }
    }
    return parse_decimal_number(expr);  // Default to decimal parsing
}

// Function to parse a binary number
int parse_binary_number(const char **expr) {
    int num = 0;
    while (**expr == '0' || **expr == '1') {
        num = (num << 1) | (**expr - '0');  // Shift left and add the new bit
        (*expr)++;
    }
    return num;
}

// Function to parse and evaluate the expression
long long evaluate(const char **expression) {
    long long result = parse_expression(expression); // Parse and evaluate the expression
    return result; // Return the result
}
uint64_t fnv1a_hash(const void *data, size_t len) {
    uint64_t hash = 0xcbf29ce484222325;  // FNV offset basis
    const uint8_t *ptr = (const uint8_t *)data;

    for (size_t i = 0; i < len; i++) {
        hash ^= ptr[i];
        hash *= 0x100000001b3;  // FNV prime
    }
    return hash;
}
// Updated parse_expression function
long long parse_expression(const char **expr) {
    long long num = 0;
    long long stack[MAX_STACK_SIZE];
    int top = -1;
    int sign = 1;

    while (**expr) {
        switch (**expr) {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                num = parse_number(expr);
                stack[++top] = sign * num;
                sign = 1;  // Reset sign after using it
                break;
            
            case '+':
                goto case_plus;

            case '-':
                // Check if this is a unary minus
                goto case_minus;

            case '/':
                goto case_divide;

            case '*':
                goto case_multiply;

            case '(':
                goto case_open_paren;

            case ')':
                goto case_close_paren;

            case '>':
                goto case_greater;

            case '<':
                goto case_less;

            case '=':
                goto case_equal;

            case '!':
                goto case_not;

            case '&':
                goto case_and;

            case '|':
                goto case_or;

            case '%':
                goto case_modulo;

            case '^':
                goto case_xor;

            case '"':
                goto case_double_quote;

            case '\'':
                goto case_single_quote;

            default:
                (*expr)++;
                break;
            continue_case:
        }
        break_case:;
    }
end_while: ;
    // Sum up all numbers in the stack
    long long result = stack[0];  // Start with first number
    for (int i = 1; i <= top; i++) {
        result += stack[i];
    }
    return result;

/* -------------------------------------------------------------------------
 * Handle addition in expressions
 * -------------------------------------------------------------------------
 */
case_plus:
    (*expr)++;
    num = evaluate(expr);
    stack[top] += num;
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle subtraction in expressions
 * -------------------------------------------------------------------------
 */
case_minus:
    if (top == -1 || (*expr)[-1] == '(') {
        sign = -1;  // Set sign to -1 for unary minus
        (*expr)++;
        goto break_case;
    }
    (*expr)++;
    // Check if the next character starts a string literal
    if (**expr == '"' || **expr == '\'') {
        // It's a string, ignore the minus and just parse the string
        goto continue_case;
    } else {
        // It's not a string, handle as a normal minus operation
        num = evaluate(expr);  // Correctly evaluate the next expression
        stack[top] -= num;     // Subtract the evaluated number from the top of the stack
    }
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle multiplication in expressions
 * -------------------------------------------------------------------------
 */
case_multiply:
    (*expr)++;
    if (**expr == '*') {
        // Handle power operator
        (*expr)++;
        num = evaluate(expr);  // Changed from parse_number to evaluate
        double base = stack[top];
        double result = pow(base, num);
        stack[top] = (int)result;  // Cast back to int after pow
    } else {
        // Handle multiplication
        num = evaluate(expr);
        stack[top] *= num;
    }
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle division in expressions
 * -------------------------------------------------------------------------
 */
case_divide:
    (*expr)++;
    // Handle division
    num = evaluate(expr);
    if (num == 0) stack[top] = INT_MAX;
    else stack[top] /= num;
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle opening a parenthesis
 * -------------------------------------------------------------------------
 */
case_open_paren:
    (*expr)++;
    num = parse_expression(expr);
    stack[++top] = sign * num;
    sign = 1;
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle closing a parenthesis
 * -------------------------------------------------------------------------
 */
case_close_paren:
    (*expr)++;
    goto end_while;

/* -------------------------------------------------------------------------
 * Handle greater or equal than in expressions
 * -------------------------------------------------------------------------
 */
case_greater:
    (*expr)++;
    if (**expr == '=') {
        (*expr)++;
        num = evaluate(expr);
        stack[top] = (stack[top] >= num) ? 1 : 0;
    } else {
        num = evaluate(expr);
        stack[top] = (stack[top] > num) ? 1 : 0;
    }
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle less or equal than in expressions
 * -------------------------------------------------------------------------
 */
case_less:
    (*expr)++;
    if (**expr == '=') {
        (*expr)++;
        num = evaluate(expr);
        stack[top] = (stack[top] <= num) ? 1 : 0;
    } else {
        num = evaluate(expr);
        stack[top] = (stack[top] < num) ? 1 : 0;
    }
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle equal comparison expressions
 * -------------------------------------------------------------------------
 */
case_equal:
    (*expr)++;
    if (**expr == '=') {
        (*expr)++;
        num = evaluate(expr);
        stack[top] = (stack[top] == num) ? 1 : 0;
    }
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle NOT equal expressions
 * -------------------------------------------------------------------------
 */
case_not:
    (*expr)++;
    if (**expr == '=') {
        (*expr)++;
        num = evaluate(expr);
        stack[top] = (stack[top] != num) ? 1 : 0;
    } else {
        // Only evaluate until the next operator
        char *next_op = strpbrk(*expr, "+-*/%|&^<>=!");
        if (next_op) {
            char saved = *next_op;
            *next_op = '\0';  // Temporarily terminate string
            num = evaluate(expr);
            *next_op = saved;  // Restore the operator
            *expr = next_op;   // Move pointer to next operator
        } else {
            num = evaluate(expr);
        }
        stack[++top] = !num;
    }
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle AND in expressions
 * -------------------------------------------------------------------------
 */
case_and:
    (*expr)++;
    num = evaluate(expr);
    stack[top] = (stack[top] && num) ? 1 : 0;  // Logical AND
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle OR in expressions
 * -------------------------------------------------------------------------
 */
case_or:
    (*expr)++;
    num = evaluate(expr);
    stack[top] = (stack[top] || num) ? 1 : 0;
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle modulo division in expressions
 * -------------------------------------------------------------------------
 */
case_modulo:
    (*expr)++;
    num = evaluate(expr);
    if (num != 0) {
        stack[top] = stack[top] % num;  // Modulo
    }
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle XOR in expressions
 * -------------------------------------------------------------------------
 */
case_xor:
    (*expr)++;
    num = evaluate(expr);
    stack[top] = stack[top] ^ num;  // Bitwise XOR
    goto break_case;

/* -------------------------------------------------------------------------
 * Handle quoted strings
 * -------------------------------------------------------------------------
 */
case_double_quote:
case_single_quote: {
    char quote = **expr;
    (*expr)++; // Skip the opening quote
    char str_buffer[256]; // Buffer for the string
    int i = 0;
    while (**expr != quote && **expr != '\0' && i < sizeof(str_buffer) - 1) {
        str_buffer[i++] = **expr; // Copy characters to buffer
        (*expr)++;
    }
    str_buffer[i] = '\0'; // Null-terminate the string
    if (**expr == quote) (*expr)++; // Skip the closing quote
    // Push the string hash onto the stack
    stack[++top] = fnv1a_hash(str_buffer, i);
  }
  goto break_case;
}

PROCEDURE(eval) {
    char *expression = GETSTRING(ARG0);
    const char *orig_expr = expression;  // Save original expression for display
    int result = evaluate((const char **) &expression);
  //  printf("EVAL: %s = %d\n", orig_expr, result);  // Use original expression
    RETURNINTX(result);
ENDPROC;
}

PROCEDURE(iff) {
    char *expression = GETSTRING(ARG0);
    const char *orig_expr = expression;  // Save original expression for display
    int result = evaluate((const char **) &expression);
    printf("IFF: %s = %d\n", orig_expr, result);  // Use original expression
    if(result==0) RETURNSTRX(GETSTRING(ARG2));  // if result = 0 then interpret as not true
    RETURNSTRX(GETSTRING(ARG1));                // anything else is true
ENDPROC;
}

void trim_brackets(char *str) {
    // Check for leading '['
    char * start=str;
    while (*start == ' ') {    // trim leading blanks
        start++; // Move the pointer forward
    }
    if (*start == '[') start++; // Move the pointer forward, if it is coded

    // Check for trailing ']'
    char *end = str + strlen(str) - 1; // Point to the last character
    while (end > start && *end == ' ') {  // trim succeeding blanks
        end--; // Move the pointer backward
    }
    if (*end == ']') end--; // drop it
 // Null-terminate the string after the last valid character
    *(end + 1) = '\0'; // Set the character after the last valid character to null

    // If the string is empty after trimming, reset it to an empty string
    if (str == end + 1) {
        str[0] = '\0'; // Set the string to empty
    } else {
      strcpy(str,start)  ;
    }

}
// Funktion zum Hinzufügen von Elementen zu einem Zielarray
PROCEDURE(add_items) {
    char *items_string = GETSTRING(ARG1); // String mit den Elementen
    SETARRAYHI(ARG0, 0); // Initialize target array

    // Check length of input string
    if (items_string == NULL) {
        RETURNINTX(0); // nothing found
    }

    // Trim brackets from the start and end of the string
    trim_brackets(items_string);

    // Tokenize the string and add items to the target array
    char *token = strtok(items_string, ","); // Tokenize using comma as delimiter
    int j = 0;
    while (token != NULL) {
        j++;
        // Append the token to the target array
        SETARRAYHI(ARG0, j); // Initialize target array
        SETSARRAY(ARG0, j-1,trim(token));
        token = strtok(NULL, ","); // Get the next token
    }
    RETURNINTX(j); // Return the count of items added
ENDPROC
}

// New PARSE function to extract variables from a template and input string
PROCEDURE(PARSE) {
    char *input_string = GETSTRING(ARG0);  // Get the input string
    char *parse_template = GETSTRING(ARG1);  // Get the parse template
    int count = 0;                          // Counter for the number of variables
    char *current_pos = input_string;        // Current position in input string

    // Initialize output arrays
    SETARRAYHI(ARG2, 0);
    SETARRAYHI(ARG3, 0);

    // Check for NULL input
    if (parse_template == NULL || input_string == NULL) {
        SETARRAYHI(ARG2, 1);
        SETSARRAY(ARG2, 0, input_string ? input_string : "");
        RETURNINT(0);
    }

    // Handle delimiter-based parsing
    char *template_copy = strdup(parse_template);
    char *token = strtok(template_copy, "'");
    int is_delimiter = 0;  // Toggle between variable names and delimiters

    while (token != NULL && count < 512) {
        token = trim(token);  // Remove whitespace

        if (strlen(token) > 0) {
            if (!is_delimiter) {
                // Store variable name
                SETARRAYHI(ARG2, count + 1);
                SETSARRAY(ARG2, count, token);

                // Get next token (delimiter)
                token = strtok(NULL, "'");
                if (token != NULL) {
                    token = trim(token);
                    // Find this delimiter in input string
                    char *next_delim = strstr(current_pos, token);

                    if (next_delim != NULL) {
                        // Extract value up to delimiter
                        int length = next_delim - current_pos;
                        char *value = (char *) malloc(length + 1);
                        strncpy(value, current_pos, length);
                        value[length] = '\0';

                        SETARRAYHI(ARG3, count + 1);
                        SETSARRAY(ARG3, count, trim(value));
                        free(value);

                        current_pos = next_delim + strlen(token);
                    } else {
                        // Last variable gets rest of string
                        SETARRAYHI(ARG3, count + 1);
                        SETSARRAY(ARG3, count, trim(current_pos));
                    }
                } else {
                    // No delimiter after variable - take rest of string
                    SETARRAYHI(ARG3, count + 1);
                    SETSARRAY(ARG3, count, trim(current_pos));
                }
                count++;
            }
            is_delimiter = !is_delimiter;
        }
        token = strtok(NULL, "'");
    }

    free(template_copy);
    RETURNINT(count);
    ENDPROC;
}
    PROCEDURE (fquoted_old) {
        char * template = GETSTRING(ARG0);
        char *start = NULL;
        const char *end = NULL;
        char quote_char = '\0';
        int i=0,j=0;
        char buffer[512];
        int count=0;
        // Search for opening quote

        while (template[i]) {
            // Skip whitespace
            while (isspace(template[i])) i++;

            if (template[i] == '\'' || template[i] == '"') {   // Quoted separator
                char quote = template[i++];
                int start = i;
                while (template[i] && template[i] != quote) i++;
                int len = i - start;
                if (len > 63) len = 63;
                SETARRAYHI(ARG1,count+1);  // set arrayhi
                SETARRAYHI(ARG2,count+1);  // set arrayhi
                strncpy(buffer, &template[start], len);
                buffer[len] = '\0';
                SETSARRAY(ARG1, count, buffer);
                SETSARRAY(ARG2, count, "S");
                count++;
                if (template[i]) i++;  // Skip closing quote
            } else if (isalpha(template[i])) {
                // Variable name
                int start = i;
                while (isalnum(template[i])) i++;
                int len = i - start;
                if (len > 63) len = 63;
                strncpy(buffer, &template[start], len);
                buffer[len] = '\0';
                SETARRAYHI(ARG1,count+1);  // set arrayhi
                SETARRAYHI(ARG2,count+1);  // set arrayhi

                SETSARRAY(ARG1, count, buffer);
                SETSARRAY(ARG2, count, "V");
                count++;
            } else {
                i++;  // Skip unknown char
            }
        }

        RETURNINTX(count)
        ENDPROC
    }

PROCEDURE(fquoted) {
    char *template = GETSTRING(ARG0);
    char buffer[512];
    int i = 0;
    int const_count = 0;
    int vname_count = 0;

    while (template[i]) {
        // Skip whitespace
        while (isspace(template[i])) i++;

        if (template[i] == '\'' || template[i] == '"') {
            // Quoted separator
            char quote = template[i++];
            int start = i;
            while (template[i] && template[i] != quote) i++;
            int len = i - start;
            if (len > 63) len = 63;
            strncpy(buffer, &template[start], len);
            buffer[len] = '\0';

            // Fill const.
            SETARRAYHI(ARG1, const_count + 1);
            SETSARRAY(ARG1, const_count, buffer);
            const_count++;
            if (template[i]) i++;  // Skip closing quote
        } else if (isalpha(template[i])) {
            // Variable name
            int start = i;
            while (isalnum(template[i])) i++;
            int len = i - start;
            if (len > 63) len = 63;
            strncpy(buffer, &template[start], len);
            buffer[len] = '\0';

            // Fill vname.
            SETARRAYHI(ARG2, vname_count + 1);
            SETSARRAY(ARG2, vname_count, buffer);
            vname_count++;
        } else {
            i++;  // Skip unknown char
        }
    }
    RETURNINTX(const_count + vname_count);
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
   ADDPROC(eval, "strings.eval", "b", ".int", "expression=.string");
   ADDPROC(iff, "strings.iff", "b", ".string", "expression=.string,true=.string,false=.string");
   ADDPROC(add_items, "strings.set_items", "b", ".int", "expose target_array=.string[],items_string=.string");
   ADDPROC(PARSE, "strings.parse", "b", ".int", "input_string=.string, parse_template=.string, expose varnames=.string[], expose varvalues=.string[]");
   ADDPROC(fquoted,      "strings.find_quoted",  "b",  ".int",   "string=.string, expose tokens=.string[],expose types=.string[]");
ENDLOADFUNCS

