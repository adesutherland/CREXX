//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

#define MAX_VARS 100
#define MAX_INDENT 10
#define MAX_VAR_NAME 32
#define MAX_LOOP_ITERATIONS 1000

// Add helper macros
#define IS_NUMERIC_VAR(idx) (vars[idx].type == VAR_TYPE_NUMBER)
#define IS_STRING_VAR(idx) (vars[idx].type == VAR_TYPE_STRING)
#define CHECK_NUMERIC_VARS(idx1, idx2) (IS_NUMERIC_VAR(idx1) && IS_NUMERIC_VAR(idx2))

// Debug switch
static int debug_mode = 1;

// Debug print macro
#define DEBUG_PRINT(...) if (debug_mode) printf(__VA_ARGS__)

typedef enum {
    VAR_TYPE_NONE = 0,
    VAR_TYPE_NUMBER = 'n',
    VAR_TYPE_STRING = 's'
} VarType;

struct Variable {
    char name[MAX_VAR_NAME];
    VarType type;
    union {
        double num_value;
        char* str_value;
    } value;
};

// Add these static function declarations at the top with the others
static int get_indent(const char* line);
static int get_var_index(const char* name);
static int is_number(const char* str);

// Add these static variables before the functions
static struct Variable vars[MAX_VARS];
static int var_count = 0;

// Add these with the other static variables at the top
static int if_stack[MAX_INDENT] = {1};
static int while_positions[MAX_INDENT] = {-1};
static int last_if_result = 0;

/* -------------------------------------------------------------------------------------
 * Bubble Sorts as a poor example not to be used
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(bubble_sort)
{
    int i, j;
    int size;

    // Check the number of arguments
    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected")

    // Get the array size
    size = GETNUMATTRS(ARG0);

    // Bubble-sort the array
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (GETSTRING(GETATTR(ARG0,j)) > GETSTRING(GETATTR(ARG0,j + 1))) {
                SWAPATTRS(ARG0, j, j + 1);
            }
        }
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Shell Sort
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (shell_sort) {
    int from, to, offset;
    int i, j, gap;
    char *val1, *order;

    to     = GETARRAYHI(ARG0);   // number of contained array items
    offset = GETINT(ARG1)-1;     // make offset
    order  = GETSTRING(ARG2);    // sort order

    from = 1;
    for (gap = to / 2; gap > 0; gap /= 2) {
        for (i = gap; i < to; i++) {
            val1 = GETSARRAY(ARG0, i);
            for (j = i; j >= gap && strcmp(GETSARRAY(ARG0,j-gap)+offset, val1+offset) > 0; j -= gap) {
                SWAPARRAY(ARG0, j, j - gap);
            }
            SETSARRAY(ARG0, j, val1);
        }
    }
    if (order[0] == 'D' || order[0] == 'd') {
        to--;        // make index to offset
        j = to / 2;      // split in halfs
        for (i = 0; i <= j; ++i) {
            SWAPARRAY(ARG0, i, to);
            to--;
        }
    }
    PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Reverse array order
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (reverse_array) {
    int i, k, to;
    to = GETARRAYHI(ARG0)-1; // make max index to offset
    k = to / 2;      // split in halfs
    for (i = 0; i <= k; ++i) {
        SWAPARRAY(ARG0, i, to);
        to--;
    }
     PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Search certain strings in array and return its index or zero
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (search_array) {
    int i, from, to;

    to  = GETARRAYHI(ARG0);
    from=GETINT(ARG2)-1;

    for (i = from; i < to; ++i) {
        if (strstr(GETSARRAY(ARG0, i),GETSTRING(ARG1))>0) {
            RETURNINT(i+1);
            PROCRETURN
        }
    }
    RETURNINT(0);
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Delete item(s) in an array and shift elements accordingly
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (delete_array) {
    int i, hi, from, to, del = 0, todel;
    hi = GETARRAYHI(ARG0) - 1; // make max index to offset
    from = GETINT(ARG1) - 1;     // get from offset
    to = GETINT(ARG2) - 1;     // get to offset
    if (from < 0 || from > hi || to < from || to < 0) {
        RETURNINT(0);
        PROCRETURN
    }
    if (from > hi) from = hi;
    if (to - from > hi) to = hi;
    todel = to - from + 1;
    while (todel >
           0) {                // position of index changes, therefore we always delete from, which is the last from+1
        REMOVEATTR(ARG0, from);
        del++;
        todel--;
    }
    RETURNINT(del);
    PROCRETURN
    ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Insert empty item(s) in an array and shift elements accordingly
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (insert_array) {
    int i, hi, from, new, del = 0, todel;
    hi = GETARRAYHI(ARG0) - 1; // make max index to offset
    from = GETINT(ARG1) - 1;     // get from offset
    new = GETINT(ARG2);       // lines to insert
    if (from < 0 || new < 1) {
        RETURNINT(0);
        PROCRETURN
    }
    if (from > hi) from = hi+1;
    for (i = from; i < from + new; ++i) {
        INSERTATTR(ARG0, i);
    }
    RETURNINT(new);
    PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Copy an entire array or a range of into a target array
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (copy_array) {
    int i, j=0, hi, from, tto,add=0;
    char * val1;
    hi = GETARRAYHI(ARG0);
    from= GETINT(ARG2)-1;
    tto = GETINT(ARG3)-1;

    if (from < 0 || from>hi || from > tto) {
        RETURNINT(0);
        PROCRETURN
    }

    if (tto>hi) tto=hi-1;

    SETARRAYHI(ARG1,tto-from+1);

    for (i = from ; i <= tto; ++i,++j) {
        val1 = GETSARRAY(ARG0, i);
        SETSARRAY(ARG1, j,val1);
        add++;
    }
    SETARRAYHI(ARG1,add);
    RETURNINT(add);
    PROCRETURN
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Merge an array into an existing one, both arrays must be sorted
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (merge_array) {
    int i, j, hi1,hi2;
    hi1 = GETARRAYHI(ARG0);
    hi2 = GETARRAYHI(ARG1);
    for (i = 0, j=0 ; i < hi1+hi2 && j<hi2; ++i) {
         if (strcmp(GETSARRAY(ARG0, i), GETSARRAY(ARG1, j)) > 0) {
            INSERTSARRAY(ARG0, i, GETSARRAY(ARG1, j));  // insert at position i a new entry
            j++;
        }
    }
    hi1=GETARRAYHI(ARG0);
    for (j = j; j < hi2; ++j,++hi1) {
        INSERTSARRAY(ARG0, hi1, GETSARRAY(ARG1, j));  // insert at position hi1 a new entry
    }
    PROCRETURN;
    RETURNINT(0);
    ENDPROC
}
/* -------------------------------------------------------------------------------------
 * list contents of an array
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (list_array)  {
    int i,from,to,hi;
    hi  = GETARRAYHI(ARG0);
    from= GETINT(ARG1);
    to  = GETINT(ARG2);
    if (to<=0) to=hi;

    if (from<1) from=1;
    if (from>hi) from=hi;
    if (to>hi || to<1) to=hi;
    printf("      Entries of String Array \n");
    printf("Entry     Data   Range %d-%d\n",from,to);
    printf("-------------------------------------------------------\n");
    for (i=from-1;i<to;i++) {
        printf("%0.7d   %s\n",i+1, GETSARRAY(ARG0,i));
    }
    printf("%d Entries\n",to);
}
/* -------------------------------------------------------------------------------------
 * Quick Sort implementation
 * -------------------------------------------------------------------------------------
 */
static void quicksort_recursive(void *array, int low, int high, int offset) {
    if (low < high) {
        // Use middle element as pivot to avoid worst case on sorted arrays
        int pivot_idx = low + (high - low) / 2;
        char *pivot = GETSARRAY(array, pivot_idx);
        
        // Move pivot to end
        SWAPARRAY(array, pivot_idx, high);
        
        int i = low - 1;
        
        // Partition
        for (int j = low; j < high; j++) {
            if (strcmp(GETSARRAY(array, j) + offset, pivot + offset) <= 0) {
                i++;
                SWAPARRAY(array, i, j);
            }
        }
        
        // Move pivot to its final position
        SWAPARRAY(array, i + 1, high);
        pivot_idx = i + 1;

        // Recursively sort the sub-arrays
        quicksort_recursive(array, low, pivot_idx - 1, offset);
        quicksort_recursive(array, pivot_idx + 1, high, offset);
    }
}

PROCEDURE(quick_sort) {
    int array, to, offset;
    char *order;

    to = GETARRAYHI(ARG0) - 1;  // Get array size (0-based index)
    offset = GETINT(ARG1) - 1;  // Get offset
    order = GETSTRING(ARG2);    // Get sort order

    // Perform quicksort
    quicksort_recursive(ARG0, 0, to, offset);

    // Handle descending order if requested
    if (order[0] == 'D' || order[0] == 'd') {
        int j = to / 2;
        for (int i = 0; i <= j; ++i) {
            SWAPARRAY(ARG0, i, to);
            to--;
        }
    }
    PROCRETURN
    ENDPROC
}

// Function declarations
static void handle_print_statement(const char* line, int line_num);
static void handle_assignment(const char* line, int line_num);
static void handle_operation(const char* line, int line_num);
static void handle_if_statement(const char* line, int* current_indent, int* if_indent, int line_num);
static void handle_else_statement(const char* line, int line_indent, int if_indent, int* current_indent, int line_num);
static void handle_while_statement(const char* line, int* current_indent, int* loop_count, int line_num, int current_line);
static int handle_indentation(int line_indent, int* current_indent, int* base_indent);
static void cleanup_variables(void);
static void safe_free(void* ptr);

// Add these macros at the top with other macros
#define HANDLE_STRING_FUNC1(func_name, len, transform) \
    if (strncmp(start, func_name"(&", len) == 0) { \
        char var2[MAX_VAR_NAME]; \
        if (sscanf(start, func_name"(&%31[^)])", var2) == 1) { \
            int src_idx = get_var_index(var2); \
            if (src_idx < 0 || !IS_STRING_VAR(src_idx)) { \
                printf("Line %d: "func_name"() requires string variable\n", line_num); \
                return; \
            } \
            vars[idx].type = VAR_TYPE_STRING; \
            vars[idx].value.str_value = safe_strdup(vars[src_idx].value.str_value); \
            transform(vars[idx].value.str_value); \
        } \
    }

#define HANDLE_NUMERIC_FUNC1(func_name, func_call, len) \
    if (strncmp(start, func_name"(&", len) == 0) { \
        char var2[MAX_VAR_NAME]; \
        if (sscanf(start, func_name"(&%31[^)])", var2) == 1) { \
            int src_idx = get_var_index(var2); \
            if (src_idx < 0) { \
                printf("Line %d: Variable not found\n", line_num); \
                return; \
            } \
            if (vars[src_idx].type != VAR_TYPE_NUMBER && vars[src_idx].type != VAR_TYPE_NONE) { \
                printf("Line %d: "func_name"() requires numeric variable\n", line_num); \
                return; \
            } \
            vars[idx].type = VAR_TYPE_NUMBER; \
            if (vars[src_idx].type == VAR_TYPE_NONE) { \
                vars[src_idx].type = VAR_TYPE_NUMBER; \
                vars[src_idx].value.num_value = 0; \
            } \
            vars[idx].value.num_value = func_call(vars[src_idx].value.num_value); \
        } \
    }

#define HANDLE_NUMERIC_FUNC2(func_name, func_call, len) \
    else if (strncmp(start, func_name"(&", len) == 0) { \
        char var1[MAX_VAR_NAME], var2[MAX_VAR_NAME]; \
        if (sscanf(start, func_name"(&%31[^,],&%31[^)])", var1, var2) == 2) { \
            int src_idx1 = get_var_index(var1); \
            int src_idx2 = get_var_index(var2); \
            if (src_idx1 < 0 || src_idx2 < 0) { \
                printf("Line %d: Variable not found\n", line_num); \
                return; \
            } \
            if ((vars[src_idx1].type != VAR_TYPE_NUMBER && vars[src_idx1].type != VAR_TYPE_NONE) || \
                (vars[src_idx2].type != VAR_TYPE_NUMBER && vars[src_idx2].type != VAR_TYPE_NONE)) { \
                printf("Line %d: "func_name"() requires numeric variables\n", line_num); \
                return; \
            } \
            vars[idx].type = VAR_TYPE_NUMBER; \
            if (vars[src_idx1].type == VAR_TYPE_NONE) { \
                vars[src_idx1].type = VAR_TYPE_NUMBER; \
                vars[src_idx1].value.num_value = 0; \
            } \
            if (vars[src_idx2].type == VAR_TYPE_NONE) { \
                vars[src_idx2].type = VAR_TYPE_NUMBER; \
                vars[src_idx2].value.num_value = 0; \
            } \
            vars[idx].value.num_value = func_call(vars[src_idx1].value.num_value, vars[src_idx2].value.num_value); \
        } \
    }

// Add these macros at the top with the other macros
#define HANDLE_LEN_FUNC(len) \
    if (strncmp(start, "len(&", len) == 0) { \
        char var2[MAX_VAR_NAME]; \
        if (sscanf(start, "len(&%31[^)])", var2) == 1) { \
            int src_idx = get_var_index(var2); \
            if (src_idx < 0 || !IS_STRING_VAR(src_idx)) { \
                printf("Line %d: len() requires string variable\n", line_num); \
                return; \
            } \
            vars[idx].type = VAR_TYPE_NUMBER; \
            vars[idx].value.num_value = strlen(vars[src_idx].value.str_value); \
        } \
    }

#define HANDLE_TRIM_FUNC(len) \
    else if (strncmp(start, "trim(&", len) == 0) { \
        char var2[MAX_VAR_NAME]; \
        if (sscanf(start, "trim(&%31[^)])", var2) == 1) { \
            int src_idx = get_var_index(var2); \
            if (src_idx < 0 || !IS_STRING_VAR(src_idx)) { \
                printf("Line %d: trim() requires string variable\n", line_num); \
                return; \
            } \
            vars[idx].type = VAR_TYPE_STRING; \
            char *str = vars[src_idx].value.str_value; \
            while (isspace(*str)) str++; \
            char *end = str + strlen(str) - 1; \
            while (end > str && isspace(*end)) end--; \
            end[1] = '\0'; \
            vars[idx].value.str_value = safe_strdup(str); \
        } \
    }

#define HANDLE_SUBSTR_FUNC(func_len) \
    else if (strncmp(start, "substr(&", func_len) == 0) { \
        char var2[MAX_VAR_NAME]; \
        int substr_pos, substr_len; \
        if (sscanf(start, "substr(&%31[^,],%d,%d)", var2, &substr_pos, &substr_len) == 3) { \
            int src_idx = get_var_index(var2); \
            if (src_idx < 0 || !IS_STRING_VAR(src_idx)) { \
                printf("Line %d: substr() requires string variable\n", line_num); \
                return; \
            } \
            vars[idx].type = VAR_TYPE_STRING; \
            char *str = vars[src_idx].value.str_value; \
            int str_len = strlen(str); \
            if (substr_pos < 1 || substr_pos > str_len || substr_len < 0) { \
                printf("Line %d: Invalid substr parameters\n", line_num); \
                return; \
            } \
            vars[idx].value.str_value = malloc(substr_len + 1); \
            strncpy(vars[idx].value.str_value, str + substr_pos - 1, substr_len); \
            vars[idx].value.str_value[substr_len] = '\0'; \
        } \
    }

// Helper functions for string transformations
static void str_to_upper(char* str) {
    for (char *p = str; *p; p++) *p = toupper(*p);
}

static void str_to_lower(char* str) {
    for (char *p = str; *p; p++) *p = tolower(*p);
}
static int evaluate_condition(const char* comp_op, double val1, double val2) {
    if (strcmp(comp_op, ">") == 0) return val1 > val2;
    if (strcmp(comp_op, "<") == 0) return val1 < val2;
    if (strcmp(comp_op, ">=") == 0) return val1 >= val2;
    if (strcmp(comp_op, "<=") == 0) return val1 <= val2;
    if (strcmp(comp_op, "==") == 0) return val1 == val2;
    if (strcmp(comp_op, "!=") == 0) return val1 != val2;
    DEBUG_PRINT("DEBUG: Unknown comparison operator '%s'\n", comp_op);
    return 0;
}
// Main pyarray function
PROCEDURE(pyarray) {
    int code_size = GETARRAYHI(ARG0);
    int loop_count = 0;
    int current_indent = 0; // Declare current_indent to track indentation levels

    // Reset static variables for new execution
    memset(if_stack, 0, sizeof(if_stack));
    if_stack[0] = 1;  // Initialize first level to true
    memset(while_positions, -1, sizeof(while_positions));
    last_if_result = 0;

    if (NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "1 argument expected: code array")

    // Process each line of code
    for (int i = 0; i < code_size; i++) {
        char* line = GETSARRAY(ARG0, i);
        
        // Move past indentation
        while (*line == ' ' || *line == '\t') line++;
        
        DEBUG_PRINT("DEBUG: Processing line %d: %s\n", i + 1, line);
        int line_indent = get_indent(line);

        // Handle assignment and operations
        if (strncmp(line, "&", 1) == 0 && strchr(line, '=')) {
            if (strchr(line, '+') || strchr(line, '-') || strchr(line, '*') || strchr(line, '/')) {
                handle_operation(line, i + 1);
                // After operation, print updated values for debugging
                int idx_i = get_var_index("i");
                int idx_max = get_var_index("max");
                if (idx_i >= 0 && idx_max >= 0) {
                    DEBUG_PRINT("DEBUG: Updated values - &i: %.2f, &max: %.2f\n", vars[idx_i].value.num_value, vars[idx_max].value.num_value);
                }
            } else {
                handle_assignment(line, i + 1);
            }
        }
        // Handle while statements
        else if (strncmp(line, "while ", 6) == 0) {
            char var1[MAX_VAR_NAME], var2[MAX_VAR_NAME], comp_op[3];
            DEBUG_PRINT("DEBUG: Parsing while statement: '%s'\n", line);
            int matched = sscanf(line, "while &%31[^ <>=!] %2[<>=!] &%31[^:]:", var1, comp_op, var2);
            DEBUG_PRINT("DEBUG: Matched %d items: var1='%s', op='%s', var2='%s'\n", matched, var1, comp_op, var2);
            if (matched == 3) {
                int idx1 = get_var_index(var1);
                int idx2 = get_var_index(var2);

                if (idx1 < 0 || idx2 < 0) {
                    printf("Line %d: Variables not found\n", i + 1);
                    return;
                }

                // Initialize variables if needed
                if (vars[idx1].type == VAR_TYPE_NONE) {
                    vars[idx1].type = VAR_TYPE_NUMBER;
                    vars[idx1].value.num_value = 0;
                }
                if (vars[idx2].type == VAR_TYPE_NONE) {
                    vars[idx2].type = VAR_TYPE_NUMBER;
                    vars[idx2].value.num_value = 0;
                }

                // Store the while statement position
                while_positions[0] = i;  // Store at base level since we don't support nested loops yet
                DEBUG_PRINT("DEBUG: Stored while position at index 0: %d\n", i);

                // Evaluate condition
                if (evaluate_condition(comp_op, vars[idx1].value.num_value, vars[idx2].value.num_value)) {
                    DEBUG_PRINT("DEBUG: Condition true, entering loop\n");
                    current_indent++;  // Increase indent level when entering loop
                } else {
                    DEBUG_PRINT("DEBUG: Condition false, skipping loop\n");
                    // Skip to the end of the loop
                    while (i + 1 < code_size && get_indent(GETSARRAY(ARG0, i + 1)) > line_indent) {
                        i++;
                    }
                    while_positions[0] = -1;  // Clear the while position if we're skipping the loop
                    continue; // Skip to the next iteration of the for loop
                }
            }
        }
        // Handle break statement
        else if (strncmp(line, "break", 5) == 0) {
            if (current_indent > 0 && while_positions[0] >= 0) {
                DEBUG_PRINT("DEBUG: Processing break statement\n");
                // Skip to the end of the loop
                while (i + 1 < code_size && get_indent(GETSARRAY(ARG0, i + 1)) > get_indent(GETSARRAY(ARG0, while_positions[0]))) {
                    i++;
                }
                while_positions[0] = -1; // Clear the while position
                current_indent--; // Exit the loop block
                continue; // Ensure we skip to the next iteration
            } else {
                printf("Line %d: break outside of loop\n", i + 1);
            }
        }
        // Handle continue statement
        else if (strncmp(line, "continue", 8) == 0) {
            if (current_indent > 0 && while_positions[0] >= 0) {
                DEBUG_PRINT("DEBUG: Processing continue statement\n");
                // Skip to the end of the current iteration
                while (i + 1 < code_size && get_indent(GETSARRAY(ARG0, i + 1)) > get_indent(GETSARRAY(ARG0, while_positions[0]))) {
                    i++;
                }
                i = while_positions[0] - 1; // Go back to while statement (minus 1 because loop will increment i)
                continue; // Ensure we skip to the next iteration
            } else {
                printf("Line %d: continue outside of loop\n", i + 1);
            }
        }
        // Handle if statements
        else if (strncmp(line, "if ", 3) == 0) {
            char var1[MAX_VAR_NAME], var2[MAX_VAR_NAME], comp_op[3];
            DEBUG_PRINT("DEBUG: Parsing if statement: '%s'\n", line);
            int matched = sscanf(line, "if &%31[^ <>=!] %2[<>=!] &%31[^:]:", var1, comp_op, var2);
            if (matched != 3) {
                matched = sscanf(line, "if %31s %2[<>=!] %31[^:]:", var1, comp_op, var2);
            }
            DEBUG_PRINT("DEBUG: Matched %d items: var1='%s', op='%s', var2='%s'\n", matched, var1, comp_op, var2);
            if (matched == 3) {
                int idx1 = get_var_index(var1);
                int idx2 = get_var_index(var2);
                double val1, val2;

                // Determine if var1 is a variable or a numeric literal
                if (idx1 >= 0) {
                    val1 = vars[idx1].value.num_value; // It's a variable
                } else {
                    val1 = atof(var1); // It's a numeric literal
                }

                // Determine if var2 is a variable or a numeric literal
                if (idx2 >= 0) {
                    val2 = vars[idx2].value.num_value; // It's a variable
                } else {
                    val2 = atof(var2); // It's a numeric literal
                }

                // Evaluate condition
                if (evaluate_condition(comp_op, val1, val2)) {
                    DEBUG_PRINT("DEBUG: If condition true, entering block\n");
                    current_indent++;  // Increase indent level when entering if block
                } else {
                    DEBUG_PRINT("DEBUG: If condition false, skipping block\n");
                    // Skip to the end of the if block
                    while (i + 1 < code_size && get_indent(GETSARRAY(ARG0, i + 1)) > get_indent(GETSARRAY(ARG0, while_positions[0]))) {
                        i++;
                    }
                }
            }
        }
        // Handle print statements
        else if (strncmp(line, "print(", 6) == 0) {
            handle_print_statement(line, i + 1);
        }
        // Handle other statements
        else {
            printf("Line %d: Invalid syntax\n", i + 1);
        }

        // Check if we're at the end of a loop body
        if (current_indent > 0 && while_positions[0] >= 0 && 
            (i + 1 >= code_size || get_indent(GETSARRAY(ARG0, i + 1)) <= get_indent(GETSARRAY(ARG0, while_positions[0])))) {
            DEBUG_PRINT("DEBUG: At end of loop body, checking condition\n");
            // Get the while statement line
            int while_pos = while_positions[0];
            char* while_line = GETSARRAY(ARG0, while_pos);
            char var1_loop[MAX_VAR_NAME], var2_loop[MAX_VAR_NAME], comp_op_loop[3];
            if (sscanf(while_line, "while &%31[^ <>=!] %2[<>=!] &%31[^:]:", var1_loop, comp_op_loop, var2_loop) == 3) {
                int idx1_loop = get_var_index(var1_loop);
                int idx2_loop = get_var_index(var2_loop);

                if (evaluate_condition(comp_op_loop, vars[idx1_loop].value.num_value, vars[idx2_loop].value.num_value)) {
                    DEBUG_PRINT("DEBUG: Condition true, continuing loop\n");
                    i = while_pos; // Jump back to the while statement
                } else {
                    DEBUG_PRINT("DEBUG: Condition false, exiting loop\n");
                    while_positions[0] = -1; // Clear the while position when done
                    current_indent--; // Decrease indent level when exiting loop
                }
            }
        }
    }

    cleanup_variables();
    PROCRETURN
    ENDPROC
}

// Implementation of the helper functions...
static void handle_print_statement(const char* line, int line_num) {
    char var_name[MAX_VAR_NAME];
    char extra;
    int matched = sscanf(line, "print(&%31[^)])%c", var_name, &extra);
    if (matched == 1 || (matched == 2 && isspace(extra))) {
        int idx = get_var_index(var_name);
        if (idx < 0) {
            printf("%d: &%s is undefined\n", line_num, var_name);
            return;
        }
        if (vars[idx].type == VAR_TYPE_NUMBER) {
            printf("%d: &%s = %.2f\n", line_num, var_name, vars[idx].value.num_value);
        } else if (vars[idx].type == VAR_TYPE_STRING) {
            printf("%d: &%s = %s\n", line_num, var_name, vars[idx].value.str_value);
        }
    }
    else {
        printf("Line %d: Invalid syntax\n", line_num);
    }
}

static void handle_operation(const char* line, int line_num) {
    char var_name[MAX_VAR_NAME];
    char op;
    char expr[MAX_VAR_NAME];
    
    // Debug: Print the operation line
    DEBUG_PRINT("DEBUG: Handling operation at line %d: %s\n", line_num, line);
    
    // Parse the operation: &var op= expr (e.g., &i += 1)
    if (sscanf(line, "&%31[^ ] %c= %31[^\n]", var_name, &op, expr) == 3) {
        // Trim whitespace from variable name
        char *end = var_name + strlen(var_name) - 1;
        while (end > var_name && isspace(*end)) end--;
        *(end + 1) = '\0';
        
        // Trim leading whitespace from expr
        char *start = expr;
        while (isspace(*start)) start++;
        
        // Debug: Print parsed components
        DEBUG_PRINT("DEBUG: Parsed operation - Variable: %s, Operator: %c, Expression: %s\n", var_name, op, start);
        
        int idx1 = get_var_index(var_name);
        if (idx1 < 0) {
            printf("Line %d: Variable &%s not found\n", line_num, var_name);
            return;
        }
        
        double operand = 0.0;
        int is_operand_var = 0;
        int idx2 = -1;
        
        if (start[0] == '&') {
            // Operand is a variable
            is_operand_var = 1;
            char operand_var[MAX_VAR_NAME];
            if (sscanf(start, "&%31[^)]", operand_var) == 1) {
                idx2 = get_var_index(operand_var);
                if (idx2 < 0) {
                    printf("Line %d: Operand variable &%s not found\n", line_num, operand_var);
                    return;
                }
                if (!IS_NUMERIC_VAR(idx2)) {
                    printf("Line %d: Operand variable &%s is not numeric\n", line_num, operand_var);
                    return;
                }
                operand = vars[idx2].value.num_value;
                DEBUG_PRINT("DEBUG: Operand from variable &%s: %f\n", operand_var, operand);
            } else {
                printf("Line %d: Invalid operand format\n", line_num);
                return;
            }
        } else if (is_number(start)) {
            // Operand is a numeric literal
            operand = atof(start);
            DEBUG_PRINT("DEBUG: Operand is numeric literal: %f\n", operand);
        } else {
            printf("Line %d: Invalid operand format\n", line_num);
            return;
        }
        
        // Perform the operation
        if (IS_NUMERIC_VAR(idx1)) {
            switch (op) {
                case '+':
                    vars[idx1].value.num_value += operand;
                    DEBUG_PRINT("DEBUG: &%s updated to %.2f\n", var_name, vars[idx1].value.num_value);
                    break;
                case '-':
                    vars[idx1].value.num_value -= operand;
                    DEBUG_PRINT("DEBUG: &%s updated to %.2f\n", var_name, vars[idx1].value.num_value);
                    break;
                case '*':
                    vars[idx1].value.num_value *= operand;
                    DEBUG_PRINT("DEBUG: &%s updated to %.2f\n", var_name, vars[idx1].value.num_value);
                    break;
                case '/':
                    if (operand == 0) {
                        printf("Line %d: Division by zero\n", line_num);
                        return;
                    }
                    vars[idx1].value.num_value /= operand;
                    DEBUG_PRINT("DEBUG: &%s updated to %.2f\n", var_name, vars[idx1].value.num_value);
                    break;
                default:
                    printf("Line %d: Unknown operator '%c'\n", line_num, op);
                    return;
            }
        }
        else if (IS_STRING_VAR(idx1)) {
            if (op == '+') {  // String concatenation
                if (is_operand_var && IS_STRING_VAR(idx2)) {
                    size_t new_len = strlen(vars[idx1].value.str_value) + strlen(vars[idx2].value.str_value) + 1;
                    char* new_str = malloc(new_len);
                    if (!new_str) {
                        printf("Line %d: Memory allocation failed\n", line_num);
                        return;
                    }
                    strcpy(new_str, vars[idx1].value.str_value);
                    strcat(new_str, vars[idx2].value.str_value);
                    safe_free(vars[idx1].value.str_value);
                    vars[idx1].value.str_value = new_str;
                    DEBUG_PRINT("DEBUG: &%s concatenated to \"%s\"\n", var_name, vars[idx1].value.str_value);
                }
                else {
                    printf("Line %d: Invalid string operand\n", line_num);
                }
            }
            else {
                printf("Line %d: Unsupported operator '%c' for string variables\n", line_num, op);
            }
        }
        else {
            printf("Line %d: Unsupported variable type for operation\n", line_num);
        }
    }
    else {
        printf("Line %d: Invalid syntax\n", line_num);
    }
}

static char* safe_strdup(const char* str) {
    if (!str) return NULL;
    char* dup = strdup(str);
    if (!dup) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    return dup;
}

static void safe_free(void* ptr) {
    if (ptr) free(ptr);
}

static int validate_variable_name(const char* name) {
    if (!name || !*name) return 0;
    if (!isalpha(*name) && *name != '_') return 0;
    for (const char* p = name + 1; *p; p++) {
        if (!isalnum(*p) && *p != '_') return 0;
    }
    return 1;
}

static int validate_array_bounds(int index, int size) {
    return index >= 0 && index < size;
}

// Add these implementations with the other helper functions
static int get_indent(const char* line) {
    int spaces = 0;
    while (*line == ' ' || *line == '\t') {
        spaces++;
        line++;
    }
    return spaces / 4;  // Using 4 spaces per indent level
}
int isnumeric(const char* varname){
    while (*varname) {
        if (!isdigit(*varname) || *varname=='+' || *varname=='-' || *varname=='.') {
            return -4;   // Non-numeric character found
        }
        varname++;
    }
    return 0;
}


static int get_var_index(const char* name) {
    // Look for existing variable
    if(isnumeric(name)==0) {
       return -1;
    }
    if (name[0] == '&') {
        name++;
    }
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return i;
        }
    }
    // Create new variable if space available
    if (var_count < MAX_VARS) {
        strncpy(vars[var_count].name, name, MAX_VAR_NAME-1);
        vars[var_count].name[MAX_VAR_NAME-1] = '\0';
        vars[var_count].type = VAR_TYPE_NONE;  // uninitialized
        vars[var_count].value.str_value = NULL;
        return var_count++;
    }
    return -1;
}

// Add cleanup_variables implementation
static void cleanup_variables(void) {
    for (int i = 0; i < var_count; i++) {
        if (vars[i].type == VAR_TYPE_STRING && vars[i].value.str_value) {
            safe_free(vars[i].value.str_value);
        }
    }
    var_count = 0;  // Reset for next run
}

static int handle_indentation(int line_indent, int* current_indent, int* base_indent) {
    // Set base indentation level if not set
    if (*base_indent == -1 && line_indent > 0) {
        *base_indent = line_indent;
    }
    
    // Normalize indentation level relative to base
    int normalized_indent = 0;
    if (*base_indent > 0) {
        normalized_indent = line_indent / *base_indent;
    }

    // Handle indentation changes
    if (normalized_indent > *current_indent + 1) {
        // Only allow one level deeper than current
        return 1;  // Just continue instead of reporting error
    }
    
    // Update current indent level
    *current_indent = normalized_indent;

    return 1;
}

static void handle_assignment(const char* line, int line_num) {
    char var_name[MAX_VAR_NAME], str_value[256];
    if (sscanf(line, "&%31[^=]=%255[^\n]", var_name, str_value) == 2) {
        // Trim whitespace from variable name
        char *end = var_name + strlen(var_name) - 1;
        while (end > var_name && isspace(*end)) end--;
        *(end + 1) = '\0';
        
        int idx = get_var_index(var_name);
        if (idx < 0) {
            printf("Line %d: Too many variables\n", line_num);
            return;
        }

        // Trim leading whitespace from value
        char *start = str_value;
        while (isspace(*start)) start++;

        // Free any existing string value
        if (vars[idx].type == VAR_TYPE_STRING && vars[idx].value.str_value) {
            safe_free(vars[idx].value.str_value);
            vars[idx].value.str_value = NULL;
        }

        // Handle different value types
        if (*start == '"' || *start == '\'') {  // String literal
            char quote = *start;
            start++;
            end = strchr(start, quote);
            if (end) {
                *end = '\0';
                vars[idx].type = VAR_TYPE_STRING;
                vars[idx].value.str_value = safe_strdup(start);
            } else {
                printf("Line %d: Unterminated string\n", line_num);
            }
        } 
        else if (is_number(start)) {  // Numeric literal
            vars[idx].type = VAR_TYPE_NUMBER;
            vars[idx].value.num_value = atof(start);
            DEBUG_PRINT("DEBUG: &%s set to %f\n", var_name, vars[idx].value.num_value);  // Debug statement
        }
        else {  // Function calls or operations
            // Check if it's an operation
            if (strchr(line, '+') || strchr(line, '-') || strchr(line, '*') || strchr(line, '/')) {
                handle_operation(line, line_num);
            }
            else {  // Function calls
                HANDLE_LEN_FUNC(5)
                HANDLE_TRIM_FUNC(6)
                HANDLE_SUBSTR_FUNC(8)
                HANDLE_STRING_FUNC1("upper", 7, str_to_upper)
                HANDLE_STRING_FUNC1("lower", 7, str_to_lower)
                HANDLE_NUMERIC_FUNC1("abs", fabs, 5)
                HANDLE_NUMERIC_FUNC1("round", round, 7)
                HANDLE_NUMERIC_FUNC1("floor", floor, 7)
                HANDLE_NUMERIC_FUNC1("ceil", ceil, 6)
                HANDLE_NUMERIC_FUNC2("pow", pow, 5)
                HANDLE_NUMERIC_FUNC2("min", fmin, 5)
                HANDLE_NUMERIC_FUNC2("max", fmax, 5)
            }
        }
    }
}

static void handle_if_statement(const char* line, int* current_indent, int* if_indent, int line_num) {
    char var1[MAX_VAR_NAME], var2[MAX_VAR_NAME], comp_op[3];
    if (sscanf(line, "if &%31[^<>=!]%2[<>=!]&%31[^:]:", var1, comp_op, var2) == 3) {
        int idx1 = get_var_index(var1);
        int idx2 = get_var_index(var2);
        
        if (idx1 < 0 || idx2 < 0) {
            printf("Line %d: Variables not found\n", line_num);
            return;
        }

        // Initialize variables if needed
        if (vars[idx1].type == VAR_TYPE_NONE) {
            vars[idx1].type = VAR_TYPE_NUMBER;
            vars[idx1].value.num_value = 0;
        }
        if (vars[idx2].type == VAR_TYPE_NONE) {
            vars[idx2].type = VAR_TYPE_NUMBER;
            vars[idx2].value.num_value = 0;
        }

        if (!IS_NUMERIC_VAR(idx1) || !IS_NUMERIC_VAR(idx2)) {
            printf("Line %d: Invalid or non-numeric variables in if statement\n", line_num);
            return;
        }

        *if_indent = *current_indent;  // Store current indent level for matching else
        (*current_indent)++;  // Increase indent level for the block
        
        // Evaluate condition and store result
        last_if_result = evaluate_condition(comp_op, vars[idx1].value.num_value, vars[idx2].value.num_value);
        if_stack[*current_indent] = last_if_result;
    }
}

static void handle_else_statement(const char* line, int line_indent, int if_indent, int* current_indent, int line_num) {
    if (line_indent != if_indent || last_if_result == -1) {
        printf("Line %d: else without matching if\n", line_num);
        return;
    }
    (*current_indent)++;
    if_stack[*current_indent] = !last_if_result;  // Execute else block if if was false
}

static void handle_while_statement(const char* line, int* current_indent, int* loop_count, int line_num, int current_line) {
    char var1[MAX_VAR_NAME], var2[MAX_VAR_NAME], comp_op[3];
    DEBUG_PRINT("DEBUG: Entering while statement at line %d\n", line_num);
    
    if (sscanf(line, "while &%31[^<>=!]%2[<>=!]&%31[^:]:", var1, comp_op, var2) == 3) {
        int idx1 = get_var_index(var1);
        int idx2 = get_var_index(var2);
        
        DEBUG_PRINT("DEBUG: While condition: &%s %s &%s\n", var1, comp_op, var2);
        DEBUG_PRINT("DEBUG: Values: %f %s %f\n", vars[idx1].value.num_value, comp_op, vars[idx2].value.num_value);
        
        if (idx1 < 0 || idx2 < 0) {
            printf("Line %d: Variables not found\n", line_num);
            return;
        }
        
        // Initialize variables if needed
        if (vars[idx1].type == VAR_TYPE_NONE) {
            vars[idx1].type = VAR_TYPE_NUMBER;
            vars[idx1].value.num_value = 0;
        }
        if (vars[idx2].type == VAR_TYPE_NONE) {
            vars[idx2].type = VAR_TYPE_NUMBER;
            vars[idx2].value.num_value = 0;
        }

        // Store position and evaluate condition
        while_positions[*current_indent] = current_line;
        if_stack[*current_indent] = 1;  // Always execute first iteration
        (*current_indent)++;
        DEBUG_PRINT("DEBUG: Stored while position at indent %d: %d\n", *current_indent-1, current_line);
    }
}

// Add the implementation with the other helper functions
static int is_number(const char* str) {
    if (!str || !*str) return 0;
    
    // Skip leading whitespace
    while (isspace(*str)) str++;
    
    // Handle optional sign
    if (*str == '+' || *str == '-') str++;
    
    // Must have at least one digit
    if (!isdigit(*str)) return 0;
    
    // Check rest of string
    int has_decimal = 0;
    while (*str) {
        if (*str == '.') {
            if (has_decimal) return 0;  // Multiple decimal points
            has_decimal = 1;
        } else if (!isdigit(*str)) {
            return 0;  // Non-digit character
        }
        str++;
    }
    
    return 1;
}

/* -------------------------------------------------------------------------------------
 * Functions to be provided to rexx
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
//      C Function, REXX namespace & name,      Option,Return Type, Arguments
//  !! Do not use "to" in the parm-list, make it for example "tto", else compile fails: "expose a = .string[],from=.int,tto=.int"
    ADDPROC(delete_array, "arrays.delete_array", "b",  ".int",      "expose a = .string[],from=.int,tto=.int");
    ADDPROC(insert_array, "arrays.insert_array", "b",  ".int",      "expose a = .string[],from=.int,new=.int");
    ADDPROC(bubble_sort,  "arrays.bubble_sort",  "b",  ".void",     "expose a = .string[]");
    ADDPROC(shell_sort,   "arrays.shell_sort",   "b",  ".void",     "expose a = .string[], offset=.int, order=.string");
    ADDPROC(reverse_array,"arrays.reverse_array","b",  ".void",     "expose a = .string[]");
    ADDPROC(search_array, "arrays.search_array", "b",  ".int",      "expose a = .string[],needle=.string,startrow=.int");
    ADDPROC(copy_array,   "arrays.copy_array",   "b",  ".int",      "expose a = .string[],b=.string[],from=.int,tto=.int");
    ADDPROC(merge_array,  "arrays.merge_array",  "b",  ".int",      "expose a = .string[],expose b=.string[]");
    ADDPROC(list_array,   "arrays.list_array",   "b",  ".void",     "expose a = .string[],from=.int,tto=.int");
    ADDPROC(quick_sort,   "arrays.quick_sort",   "b",  ".void",     "expose a = .string[], offset=.int, order=.string");
    ADDPROC(pyarray, "arrays.pyarray", "b", ".void", "expose code = .string[]");
ENDLOADFUNCS
