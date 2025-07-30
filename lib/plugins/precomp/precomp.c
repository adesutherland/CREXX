//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "crexxpa.h"  // crexx/pa - Plugin Architecture header file

// Debug switch
static int debug_mode = 1;
#define DEBUG_PRINT(...) if (debug_mode) printf(__VA_ARGS__)

// Add these static function declarations at the top with the others

void searchReplace(char *str, char search, char replace) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {  // Loop until the end of the string
        if (str[i] == search) {
            str[i] = replace;  // Replace the character
        }
    }
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

PROCEDURE (sort_bylen) {
    int from, to;
    int i, j, gap;
    int val1;
    to = GETARRAYHI(ARG0);   // number of contained array items

    from = 1;
    for (gap = to / 2; gap > 0; gap /= 2) {
        for (i = gap; i < to; i++) {
            val1 = strlen(GETSARRAY(ARG0, i));
            for (j = i; j >= gap && strlen(GETSARRAY(ARG0, j - gap))<val1; j -= gap) {
                SWAPARRAY(ARG0, j, j - gap);
                SWAPARRAY(ARG1, j, j - gap);
                SWAPARRAY(ARG2, j, j - gap);
            }
         }
    }
}
/* -------------------------------------------------------------------------------------
 * Search certain strings in array and return its index or zero
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (search_array) {
    int i, from, to;

    to = GETARRAYHI(ARG0);
    from = GETINT(ARG2) - 1;
    int match = GETINT(ARG3);
    if (match == 0) {   // substring match sufficient
        for (i = from; i < to; ++i) {
            if (strstr(GETSARRAY(ARG0, i), GETSTRING(ARG1)) > 0) {
                RETURNINTX(i + 1);
            }
        }
    } else {
        for (i = from; i < to; ++i) {  // full string match required
            if (strcmp(GETSARRAY(ARG0, i), GETSTRING(ARG1)) == 0) {
                RETURNINTX(i + 1);
            }
        }
    }
    RETURNINT(0);
    ENDPROC
}

/* -------------------------------------------------------------------------------------
 * Drop entire Array
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (drop_array) {
    int i, hi;
    hi = GETARRAYHI(ARG0) - 1; // make max index to offset
    for (i = 1; i < hi; ++i) {
        REMOVEATTR(ARG0, i);
    }
    SETARRAYHI(ARG0, 1);  // reset arrayhi
    RETURNINTX(hi);
ENDPROC

}
/* -------------------------------------------------------------------------------------
 * Insert empty item(s) in an array and shift elements accordingly
 * the line number is the first which is shifted,
 * this means the empty lines are added prior to this line
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (insert_array) {
    int i, hi, from, new;
    hi = GETARRAYHI(ARG0) - 1; // make max index to offset
    from = GETINT(ARG1) - 1;     // get from offset
    new = GETINT(ARG2);       // lines to insert
    if (from < 0 || new < 1) {
        RETURNINTX(0);
    }
    if (from > hi) from = hi+1;
    for (i = from; i < from + new; ++i) {
        INSERTATTR(ARG0, i);
    }
   RETURNINTX(new);
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
 * list contents of an array
 * -------------------------------------------------------------------------------------
 */
PROCEDURE (list_array)  {
    int i,from,to,hi;
    hi  = GETARRAYHI(ARG0);
 //   if (hi==1 && strlen(GETSARRAY(ARG0,1))==0 ) hi=0;
    from= GETINT(ARG1);
    to  = GETINT(ARG2);
    char * hdr=GETSTRING(ARG3);
    if (to<=0) to=hi;
    if (from<1) from=1;
    if (from>hi) from=hi;
    if (to>hi || to<1) to=hi;
    if (hdr[0]=='\0') printf("      Entries of String Array \n");
    else printf("%s\n",hdr);
    printf("Entry     Data   Range %d-%d\n",from,to);
    printf("-------------------------------------------------------\n");
    if(hi<=0) printf("Array contains no Entries\n");
    else {
       for (i=from-1;i<to;i++) {
           printf("%0.7d   %s\n",i+1, GETSARRAY(ARG0,i));
       }
       printf("%d Entries\n",to);
    }
}

PROCEDURE(readall) {
    int lines = 1, llen, amax = 0, amaxl, maxlines;
    searchReplace(GETSTRING(ARG1), '\\', '/');
    maxlines = GETINT(ARG2);
    FILE *file = fopen(GETSTRING(ARG1), "r");
    if (file == NULL) {
        RETURNINT(-8);
        PROCRETURN
    }
    char line[32000];      // 32k should be enough
    amaxl = 250;             // define new junk to allow additional lines in exceeded array
    while (fgets(line, sizeof(line), file) != NULL) {
        llen = strlen(line);
        if (line[llen - 1] == '\n') line[llen - 1] = '\0';
        if (amax == 0) {
            amax = amaxl;
            SETARRAYHI(ARG0, GETARRAYHI(ARG0) + amaxl);
        }
        SETSARRAY(ARG0, lines - 1, line);   // push into array

        if (maxlines > 0 && lines >= maxlines) break;
        lines++;
        amax--;
    }
    SETARRAYHI(ARG0, lines);  // adjust arrayhi
    RETURNINT(lines);
    PROCRETURN
    ENDPROC
}

PROCEDURE(writeall) {
    int lines = 0, tto, i, maxlines;
    searchReplace(GETSTRING(ARG1), '\\', '/');
    FILE *file = fopen(GETSTRING(ARG1), "w");
    if (file == NULL) {
        RETURNINT(-8);
        PROCRETURN
    }
    tto = GETARRAYHI(ARG0);
    maxlines = GETINT(ARG2);
    for (i = 0; i < tto; ++i) {
        if (lines > 0) fputs("\n", file);   // complete last record, avoids NL for last record
        fputs(GETSARRAY(ARG0, i), file);
        lines++;
        if (maxlines > 0 && lines >= maxlines) break;
    }
    fclose(file);
    RETURNINT(lines);
    PROCRETURN
    ENDPROC
}
char *strcasestr(const char *haystack, const char *needle) {
    if (!*needle)
        return (char *)haystack;

    for (; *haystack; ++haystack) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            ++h;
            ++n;
        }
        if (!*n)
            return (char *)haystack;
    }
    return NULL;
}

/* -------------------------------------------------------------------------------------
* Portable strupr() because that is windows-only
* -------------------------------------------------------------------------------------
*/

char *strupr_portable(char *s) {
    char *p = s;
    while (*p) {
        *p = toupper((unsigned char)*p);
        p++;
    }
    return s;
}
/* -------------------------------------------------------------------------------------
* Check if line contains a macro call
* -------------------------------------------------------------------------------------
*/
PROCEDURE (hasmacro) {
    int i,mmax, from;
    char * line= GETSTRING(ARG0);
    mmax = GETARRAYHI(ARG1);   // number of contained array items
    from = GETINT(ARG2)-1;
    if (from<0) from=0;
    if (from>=mmax) RETURNINTX(0);
    if (strstr(line,"(")==0) RETURNINTX(0);  // there is no need to go over a macro search if there isno macro call having a ( sign
    while (isspace((unsigned char)*line)) {
        line++;
    }
    if (line[0]=='#' & line[1]=='#') {
       RETURNINTX(0)
    };
    if (line[0]=='/' & line[1]=='*') {
        RETURNINTX(0)
    };
    char * dupline = strdup(line);
    dupline= strupr_portable(dupline);
    for (i = from; i<mmax; i++) {
         if (strstr(dupline, GETSARRAY(ARG1, i)) > 0) {
            RETURNINTX(i + 1);
        }
    }
    RETURNINTX(0);
    PROCRETURN
ENDPROC
}

// Insert 'needle' into 'haystack' at position 'at', replacing 'len' characters
PROCEDURE (insertat) {
// char* insertAt(const char* haystack, const char* needle, int at, int len) {
    char * needle  = GETSTRING(ARG0);
    char * haystack= GETSTRING(ARG1);
    int at =  GETINT(ARG2)-1;
    int len = GETINT(ARG3) ;
 //   printf("111 needle '%s' haystack '%s' at %d len %d\n",needle,haystack,at,len);
    int haystack_len = strlen(haystack);
    int needle_len = strlen(needle);

    // Ensure valid indices
    if (at < 0) at = 0;
    if (at > haystack_len-1) at = haystack_len;

 // Bounds check
    if (at < 0) at = 0;
    if (at > haystack_len) at = haystack_len;  // Insert at end if at > length
    if (at + len > haystack_len) len = haystack_len - at;

    // Length of resulting string
    int result_len = haystack_len - len + needle_len;

    // Allocate result string
    char* result = malloc(result_len + 1);
    if (!result) RETURNSTRX("");

    // Copy left part
    strncpy(result, haystack, at);

    // Copy needle
    strcpy(result + at, needle);

    // Copy right part
    strcpy(result + at + needle_len, haystack + at + len);
    RETURNSTR(result);
    free(result);
    ENDPROC
}
/* -------------------------------------------------------------------------------------
* search in array a certain string, up to 3 strings are possible
* -------------------------------------------------------------------------------------
*/
PROCEDURE (fsearch) {
    int i,mmax, from;
    mmax = GETARRAYHI(ARG0);   // number of contained array items
    from = GETINT(ARG1)-1;
    if (from<0) from=0;
    if (from>=mmax) RETURNINTX(0);
    SETINT(ARG5, 0);
    char * ustr1= GETSTRING(ARG2);
    char * ustr2= GETSTRING(ARG3);
    char * ustr3= GETSTRING(ARG4);
    char * haystack;
    for (i = from; i < mmax; i++) {
        haystack = strupr_portable(strdup(GETSARRAY(ARG0, i)));
        while (isspace((unsigned char)*haystack)) haystack++;
        if (ustr1[0] != '\0')
            if (strstr(haystack, GETSTRING(ARG2)) == haystack) {
                SETINT(ARG5, 1);
                RETURNINTX(i + 1);
            }
        if (ustr2[0] != '\0')
            if (strstr(haystack, GETSTRING(ARG3)) == haystack) {
                SETINT(ARG5, 2);
                RETURNINTX(i + 1);
            }
        if (ustr3[0] != '\0')
            if (strstr(haystack, GETSTRING(ARG4)) == haystack) {
                SETINT(ARG5, 3);
                RETURNINTX(i + 1);
            }
    }
    RETURNINTX(0);
    PROCRETURN
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
            printf("const %d %s\n",const_count,buffer );

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

/* -------------------------------------------------------------------------------------
* search in array a certain string, up to 3 strings are possible
* -------------------------------------------------------------------------------------
*/
PROCEDURE (xlog) {
    char * xlog=GETSTRING(ARG0);
    printf("XLOG %s\n",xlog);
}

// Helper to check if a string is properly quoted
int is_properly_quoted(const char* str) {
    size_t len = strlen(str);
    if (len < 2) return 0;

    char quote = str[0];
    if ((quote != '\'' && quote != '"') || str[len - 1] != quote)
        return 0;

    // Check for valid escaping inside
    for (size_t i = 1; i < len - 1; ++i) {
        if (str[i] == quote) {
            if (i + 1 >= len - 1 || str[i + 1] != quote)
                return 0;  // not escaped by doubling
            i++;  // skip the second quote
        }
    }
    return 1;  // properly quoted
}
// Quote a string safely for use in REXX
PROCEDURE(safe_quote) {
    char* input=GETSTRING(ARG0) ;
    if (is_properly_quoted(input)) {
        RETURNSTRX(strdup(input));  // return as-is
    }

    int has_single = strchr(input, '\'') != NULL;
    int has_double = strchr(input, '"') != NULL;

    const char quote = has_single && !has_double ? '"' : '\'';  // safest choice

 // Estimate worst-case size (every quote gets doubled + outer quotes + null terminator)
    size_t len = strlen(input);
    char* result = malloc(2 * len + 3);  // worst case: all quotes doubled + 2 outer + '\0'
    char* p = result;
    *p++ = quote;
    for (size_t i = 0; i < len; ++i) {
        if (input[i] == quote) *p++ = quote;  // double the quote
        *p++ = input[i];
    }
    *p++ = quote;
    *p = '\0';
    RETURNSTRX(result);
ENDPROC
}
PROCEDURE(fpos) {
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
LOADFUNCS
   ADDPROC(insert_array, "precomp.insert_array", "b",  ".int",   "expose a = .string[],from=.int,new=.int");
   ADDPROC(shell_sort,   "precomp.shell_sort",   "b",  ".void",  "expose a = .string[], offset=.int, order=.string");
   ADDPROC(sort_bylen,   "precomp.sort_bylen",   "b",  ".void",  "expose a = .string[],expose b = .string[],expose c = .string[]");
   ADDPROC(drop_array,   "precomp.drop_array",   "b",  ".int",   "expose a = .string[]");
   ADDPROC(search_array, "precomp.search_array", "b",  ".int",   "expose a = .string[],needle=.string,startrow=.int,match=.int");
   ADDPROC(copy_array,   "precomp.copy_array",   "b",  ".int",   "expose a = .string[],b=.string[],from=.int,tto=.int");
   ADDPROC(list_array,   "precomp.list_array",   "b",  ".void",  "expose a = .string[],from=.int,tto=.int,hdr=.string");
   ADDPROC(hasmacro,     "precomp.hasmacro",     "b",  ".int",   "line=.string,maclist=.string[],from=.int");
   ADDPROC(readall,      "precomp.readall",      "b",  ".int",   "expose array=.string[],expose file=.string,arg2=.int");
   ADDPROC(writeall,     "precomp.writeall",     "b",  ".int",   "expose array=.string[],file=.string,arg2=.int");
   ADDPROC(insertat,     "precomp.insertatc",    "b",  ".string","haystack = .string,needle=.string,offset=.int,len=.int");
   ADDPROC(fsearch,      "precomp.fsearch",      "b",  ".int",   "expose array=.string[],pos=.int,str1=.string,str2=.string,str3=.string,expose item=.int");
   ADDPROC(fquoted,      "precomp.find_quoted",  "b",  ".int",   "string=.string, expose tokens=.string[],expose types=.string[]");
   ADDPROC(xlog,         "precomp.xlog",         "b",  ".void",  "string = .string");
   ADDPROC(safe_quote,   "precomp.safe_quote",   "b",  ".string","string = .string");
   ADDPROC(fpos,         "precomp.fpos",         "b",  ".int","string = .string,substring=.string,offset=.int");
   ADDPROC(fpos,         "precomp.fpos",         "b",  ".int","string = .string,substring=.string,offset=.int");

ENDLOADFUNCS

