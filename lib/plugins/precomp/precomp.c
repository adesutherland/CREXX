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

static inline void searchReplace(char *str, char search, char replace) {
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
    int to, i, j, gap, val1;
    to = GETARRAYHI(ARG0);   // number of contained array items

    for (gap = to / 2; gap > 0; gap /= 2) {
        for (i = gap; i < to; i++) {
            val1 = strlen(GETSARRAY(ARG0, i));
            for (j = i; j >= gap && strlen(GETSARRAY(ARG0, j - gap))<val1; j -= gap) {
                SWAPARRAY(ARG0, j, j - gap);
                SWAPARRAY(ARG1, j, j - gap);
                SWAPARRAY(ARG2, j, j - gap);
                SWAPARRAY(ARG3, j, j - gap);
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
    SETARRAYHI(ARG0, 0);  // reset arrayhi
    RETURNINTX(hi);
    ENDPROC

}
/* -------------------------------------------------------------------------------------
 * Insert empty item(s) in an array and shift elements accordingly
 * the line number is the first which is shifted,
 * this means the empty lines are added prior to this line
 * !! it does not mean add empty line after lino !!
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
    if (hi==1 && strlen(GETSARRAY(ARG0,0))==0 ) hi=0;
    from= GETINT(ARG1);
    to  = GETINT(ARG2);
    char * hdr=GETSTRING(ARG3);
    if (to<=0) to=hi;
    if (from<1) from=1;
    if (from>hi) from=hi;
    if (to>hi || to<1) to=hi;
 //   printf("list array %d %d\n",from,hi);
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
    fclose(file);
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
/* -------------------------------------------------------------------------------------
* Portable strupr() because that is windows-only
* -------------------------------------------------------------------------------------
*/

static char *strupr_portable(char *s) {
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
    int i, mmax, from, result;
    char * dupline;
    char * line= GETSTRING(ARG0);
    mmax = GETARRAYHI(ARG1);   // number of contained array items
    from = GETINT(ARG2)-1;
    if (from<0) from=0;
    if (from>=mmax) RETURNINTX(0);
  //  no more check for ( in macro call as we allow macros without it
  //  if (strstr(line,"(")==0) RETURNINTX(0);  // there is no need to go over a macro search if there isno macro call having a ( sign
    while (isspace((unsigned char)*line)) {
        line++;
    }
    if (line[0]=='#' & line[1]=='#') {
        RETURNINTX(0)
    };
    if (line[0]=='/' & line[1]=='*') {
        RETURNINTX(0)
    };
    dupline = strdup(line);
    dupline= strupr_portable(dupline);
    result = 0;
    for (i = from; i<mmax; i++) {
        if (strstr(dupline, GETSARRAY(ARG1, i)) > 0) {
            result = i + 1;
            break;
        }
    }
    free(dupline);
    RETURNINTX(result);
    PROCRETURN
    ENDPROC
}

// Insert 'needle' into 'haystack' at position 'at', replacing 'len' characters
PROCEDURE (insertat) {
    char * needle;
    char * haystack;
    int at;
    int len;
    int haystack_len;
    int needle_len;
    int result_len;
    char* result;

    needle  = GETSTRING(ARG0);
    haystack= GETSTRING(ARG1);
    at =  GETINT(ARG2)-1;
    len = GETINT(ARG3) ;
    haystack_len = strlen(haystack);
    needle_len = strlen(needle);

    // Ensure valid indices
    if (at < 0) at = 0;
    if (at > haystack_len-1) at = haystack_len;

    // Bounds check
    if (at < 0) at = 0;
    if (at > haystack_len) at = haystack_len;  // Insert at end if at > length
    if (at + len > haystack_len) len = haystack_len - at;

    // Length of resulting string
    result_len = haystack_len - len + needle_len;

    // Allocate result string
    result = malloc(result_len + 1);
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
    int i, mmax, from, result;
    char *ustr1, *ustr2, *ustr3;
    char *orig_haystack, *haystack;

    mmax = GETARRAYHI(ARG0);
    from = GETINT(ARG1) - 1;
    if (from < 0) from = 0;
    if (from >= mmax) RETURNINTX(0);
    SETINT(ARG5, 0);

    ustr1 = strdup(GETSTRING(ARG2));
    ustr2 = strdup(GETSTRING(ARG3));
    ustr3 = strdup(GETSTRING(ARG4));
    strupr_portable(ustr1);
    strupr_portable(ustr2);
    strupr_portable(ustr3);

    result = 0;
    for (i = from; i < mmax; i++) {
        orig_haystack = strdup(GETSARRAY(ARG0, i));
        haystack = orig_haystack;
        strupr_portable(haystack);
        while (isspace((unsigned char)*haystack)) haystack++;

        if (ustr1[0] != '\0' && strstr(haystack, ustr1) == haystack) {
            SETINT(ARG5, 1);
            result = i + 1;
        } else if (ustr2[0] != '\0' && strstr(haystack, ustr2) == haystack) {
            SETINT(ARG5, 2);
            result = i + 1;
        } else if (ustr3[0] != '\0' && strstr(haystack, ustr3) == haystack) {
            SETINT(ARG5, 3);
            result = i + 1;
        }
        free(orig_haystack);
        if (result != 0) break;
    }
    free(ustr1);
    free(ustr2);
    free(ustr3);
    RETURNINTX(result);
    ENDPROC
}

PROCEDURE (ffind) {
    int i, mmax, from, result;
    char *ustr, *haystack;
    mmax = GETARRAYHI(ARG0);
    from = GETINT(ARG1) - 1;
    if (from < 0) from = 0;
    if (from >= mmax) RETURNINTX(0);

    ustr = strdup(GETSTRING(ARG2));
    strupr_portable(ustr);

    result = 0;
    for (i = from; i < mmax; i++) {
        haystack = strdup(GETSARRAY(ARG0, i));
        strupr_portable(haystack);
        if (strstr(haystack, ustr) != NULL) {
            result = i + 1;
        }
        free(haystack);
        if (result != 0) break;
    }
    free(ustr);
    RETURNINTX(result);
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
            char quote;
            int start, len;
            quote = template[i++];
            start = i;
            while (template[i] && template[i] != quote) i++;
            len = i - start;
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
            int start, len;
            start = i;
            while (isalnum(template[i])) i++;
            len = i - start;
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

/*
 * Splits ARG0 on commas, but ignores commas inside quotes (' or ")
 * and inside (nested) parentheses. Trims leading/trailing whitespace
 * per token. Doubled quotes inside a quoted string are treated as
 * literal quotes and kept (e.g., "He said ""hi""" -> He said "hi").
 *
 * OUTPUT:
 *   - Tokens are written to ARG1 as a sequential array, following the
 *     same 0-based convention used in your existing code snippet:
 *         SETSARRAY(ARG1, 0, "first")
 *         SETSARRAY(ARG1, 1, "second")
 *         ...
 *   - The function RETURNs the number of tokens (int).
 *
 * If you prefer 1-based "stem" semantics (.1..n and .0=n), set
 *   #define INDEX_BASE 1
 * below and see the comment in emit_final_count().
 */

/* === Your bridge macros (already present in your project) ===
   GETSTRING(ARGi)       -> const char*
   SETARRAYHI(ARGi, n)   -> set high bound/capacity
   SETSARRAY(ARGi, idx, cstr) -> set element (idx) to C string
   RETURNINTX(n)         -> return int from PROCEDURE
   PROCEDURE(name) { ... ENDPROC }
   (Leave includes for those macros as they are in your project.)
*/


/* ---------- small dynamic buffer helpers (C99) ---------- */
static void append_char(char **buf, int *len, int *cap, char ch) {
    if (*len + 1 >= *cap) {
        int newcap;
        char *nbuf;
        newcap = (*cap == 0) ? 128 : (*cap * 2);
        nbuf = (char *)realloc(*buf, (size_t)newcap);
        if (!nbuf) {
            /* Out of memory: drop char and keep going, or abort as you prefer */
            return;
        }
        *buf = nbuf;
        *cap = newcap;
    }
    (*buf)[(*len)++] = ch;
}

static void reset_token(char **buf, int *len) {
    *len = 0;
    if (*buf) (*buf)[0] = '\0';
}

/* Emit trimmed substring of token[0..tlen) into ARG1 at position (INDEX_BASE + *n) */
static void emit_trimmed(char *token, int tlen, int *n, char * outarg) {
    int start = 0, end = tlen;
    int outlen, idx;
    char *out;

    /* trim left/right whitespace */
    while (start < end && isspace((unsigned char)token[start])) start++;
    while (end > start && isspace((unsigned char)token[end - 1])) end--;

    /* if you want to KEEP empty fields (e.g., "a,,b"), remove this guard */
    if (end <= start) {
        return; /* drop empty after trimming (matches your REXX default behavior) */
    }

    outlen = end - start;
    out = (char *)malloc((size_t)outlen + 1);
    if (!out) return;

    memcpy(out, token + start, (size_t)outlen);
    out[outlen] = '\0';

    /* grow high bound and store */
    idx = (*n);
    SETARRAYHI(outarg, idx + 1);
    SETSARRAY(outarg, idx, out);

    free(out);
    (*n)++;
}

/* -------------------- main procedure -------------------- */
PROCEDURE(splitargs) {
    const char *s = GETSTRING(ARG0);
    char *token = NULL;
    int tlen = 0, tcap = 0;

    int i = 0;
    int count = 0;     /* number of emitted tokens */
    int depth = 0;     /* parenthesis nesting */
    int inq = 0;       /* inside quotes? */
    char qchar = 0;    /* current quote char */

    while (s[i]) {
        char c;
        c = s[i];
        if (inq) {
            /* inside a quoted string: copy verbatim and handle doubled quotes */
            append_char(&token, &tlen, &tcap, c);
            if (c == qchar) {
                if (s[i + 1] == qchar) {
                    /* doubled quote -> literal quote char; append one more and skip next */
                    append_char(&token, &tlen, &tcap, qchar);
                    i += 2;
                    continue;
                } else {
                    inq = 0;
                    qchar = 0;
                }
            }
            i++;
            continue;
        }
        /* not in quotes */
        if (c == ',') {
            if (depth == 0) {
                /* split point */
                emit_trimmed(token, tlen, &count, ARG1);
                reset_token(&token, &tlen);
                i++;
                continue;
            }
            /* else: literal comma inside parentheses */
        } else if (c == '\'' || c == '"') {
            inq = 1;
            qchar = c;
        } else if (c == '(') {
            depth++;
        } else if (c == ')') {
            if (depth > 0) depth--; /* ignore stray ')' */
        }

        append_char(&token, &tlen, &tcap, c);
        i++;
    }

    /* flush last token; to KEEP trailing empty fields ("a,b,"), remove the guard in emit_trimmed */
    emit_trimmed(token, tlen, &count, ARG1);

    if (token) free(token);
    RETURNINTX(count);
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
    size_t len;
    size_t i;
    char quote;
    len = strlen(str);
    if (len < 2) return 0;

    quote = str[0];
    if ((quote != '\'' && quote != '"') || str[len - 1] != quote)
        return 0;

    // Check for valid escaping inside
    for (i = 1; i < len - 1; ++i) {
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
    char* input;
    int has_single;
    int has_double;
    char quote;
    size_t len;
    size_t i;
    char* result;
    char* p;

    input=GETSTRING(ARG0) ;
    if (is_properly_quoted(input)) {
        RETURNSTRX(input);  // return as-is
    }

    has_single = strchr(input, '\'') != NULL;
    has_double = strchr(input, '"') != NULL;

    quote = has_single && !has_double ? '"' : '\'';  // safest choice

    // Estimate worst-case size (every quote gets doubled + outer quotes + null terminator)
    len = strlen(input);
    result = malloc(2 * len + 3);  // worst case: all quotes doubled + 2 outer + '\0'
    p = result;
    *p++ = quote;
    for (i = 0; i < len; ++i) {
        if (input[i] == quote) *p++ = quote;  // double the quote
        *p++ = input[i];
    }
    *p++ = quote;
    *p = '\0';
    RETURNSTR(result);
    free(result);
    PROCRETURN;
    ENDPROC;
}
// this is a case-insensitive version of the pos function
PROCEDURE(fpos) {
    char *substring;
    char *wordstring;
    int offset;
    int result;
    char *found;

    substring  = strdup(GETSTRING(ARG0));
    wordstring = strdup(GETSTRING(ARG1));
    offset = GETINT(ARG2) - 1;
    result = 0;

    if (substring && wordstring && offset >= 0) {
        strupr_portable(substring);
        strupr_portable(wordstring);
        if (offset < (int)strlen(wordstring)) {
            found = strstr(wordstring + offset, substring);
            if (found != NULL) {
                result = (int)(found - wordstring + 1);
            }
        }
    } else if (offset < 0) {
        result = -1;
    }

    if (substring) free(substring);
    if (wordstring) free(wordstring);
    RETURNINTX(result);
    ENDPROC;
}
char myArray[10][64] = { { '\0' } };
PROCEDURE(templist) {
    char mode = GETSTRING(ARG0)[0];
    int  index= GETINT(ARG1)-1;
    char *content = GETSTRING(ARG2);
    if (index < 0 || index >= 10) RETURNSTRX("");
    if(mode=='G' || mode=='g')  RETURNSTRX(myArray[index]);
 // put mode
    strncpy(myArray[index], content, 63);
    myArray[index][63] = '\0';
    RETURNSTRX(""); // Return 0 if the substring is not found
    ENDPROC;
}

/*
 * Transform:
 *   Fred.bert.name.indx
 * into:
 *   "Fred."bert"."name"."indx"
 *
 * Contract:
 *   - returns heap-allocated string (caller must free), or NULL on invalid input
 *   - never returns borrowed pointer
 */

static char *quote_stem_path(char *in) {
    size_t in_len;
    const char *dot;
    size_t root_len;
    size_t cap;
    char *out;
    size_t w;
    const char *p;
    const char *next;
    size_t seg_len;

    in_len = strlen(in);
    if (in_len == 0) return NULL;

    dot = strchr(in, '.');
    if (!dot) {
        return strdup(in);
    }

    root_len = (size_t) (dot - in);
    if (root_len == 0) return NULL; /* ".a" invalid */

    /* Max possible expansion: each segment char stays as-is, each dot becomes ||"."||
       Plus prefix "root." (root_len + 3)
    */
    cap = in_len * 8 + 32;
    out = (char *) malloc(cap);
    if (!out) return NULL;

    w = 0;
    /* Prefix: "Root." */
    out[w++] = '"';
    memcpy(out + w, in, root_len);
    w += root_len;
    out[w++] = '.';
    out[w++] = '"';

    /* Tail segments */
    p = dot + 1;

    while (*p) {
        next = strchr(p, '.');
        seg_len = next ? (size_t) (next - p) : strlen(p);

        if (seg_len == 0) {
            free(out);
            return NULL;
        }

        /* Concatenate segment: ||seg */
        out[w++] = '|';
        out[w++] = '|';
        memcpy(out + w, p, seg_len);
        w += seg_len;

        if (next) {
            /* More segments follow, add dot: ||"." */
            out[w++] = '|';
            out[w++] = '|';
            out[w++] = '"';
            out[w++] = '.';
            out[w++] = '"';
        }

        if (!next) break;
        p = next + 1;
    }

    out[w] = '\0';
    return out;
}


PROCEDURE(stemquote)
{
    char *input;
    char *out;
    // ARG0: input string
    input = GETSTRING(ARG0);
    if (!input || !*input) RETURNSTRX("");    // no input no stem

    out = quote_stem_path(input);
    if (!out) {
        RETURNSTRX(input); // invalid path: return and let rxpp decided what to do
    }
    RETURNSTR(out);
    if(input != out) free(out);   // if input=output ptr don't removee it
    ENDPROC
}


LOADFUNCS
    ADDPROC(insert_array, "precomp.insert_array", "b",  ".int",   "expose a = .string[],from=.int,new=.int");
    ADDPROC(shell_sort,   "precomp.shell_sort",   "b",  ".void",  "expose a = .string[], offset=.int, order=.string");
    ADDPROC(sort_bylen,   "precomp.sort_bylen",   "b",  ".void",  "expose a = .string[],expose b = .string[],expose c = .string[],expose d = .int[]");
    ADDPROC(drop_array,   "precomp.drop_array",   "b",  ".int",   "expose a = .string[]");
    ADDPROC(search_array, "precomp.search_array", "b",  ".int",   "expose a = .string[],needle=.string,startrow=.int,match=.int");
    ADDPROC(copy_array,   "precomp.copy_array",   "b",  ".int",   "expose a = .string[],b=.string[],from=.int,tto=.int");
    ADDPROC(list_array,   "precomp.list_array",   "b",  ".void",  "expose a = .string[],from=.int,tto=.int,hdr=.string");
    ADDPROC(hasmacro,     "precomp.hasmacro",     "b",  ".int",   "line=.string,maclist=.string[],from=.int");
    ADDPROC(readall,      "precomp.readall",      "b",  ".int",   "expose array=.string[],expose file=.string,arg2=.int");
    ADDPROC(writeall,     "precomp.writeall",     "b",  ".int",   "expose array=.string[],file=.string,arg2=.int");
    ADDPROC(insertat,     "precomp.insertatc",    "b",  ".string","haystack = .string,needle=.string,offset=.int,len=.int");
    ADDPROC(fsearch,      "precomp.fsearch",      "b",  ".int",   "expose array=.string[],pos=.int,str1=.string,str2=.string,str3=.string,expose item=.int");
    ADDPROC(ffind,        "precomp.ffind",        "b",  ".int",   "expose array=.string[],pos=.int,str1=.string");
    ADDPROC(fquoted,      "precomp.find_quoted",  "b",  ".int",   "string=.string, expose tokens=.string[],expose types=.string[]");
    ADDPROC(splitargs,    "precomp.splitargs",    "b",  ".int",   "string=.string, expose tokens=.string[]");
    ADDPROC(xlog,         "precomp.xlog",         "b",  ".void",  "string = .string");
    ADDPROC(safe_quote,   "precomp.safe_quote",   "b",  ".string","string = .string");
    ADDPROC(fpos,         "precomp.fpos",         "b",  ".int","string = .string,substring=.string,offset=.int");
    ADDPROC(templist,     "precomp.templist",     "b",  ".string","mode=.string,index=.int,string=.string");
    ADDPROC(stemquote,    "precomp.stemquote",    "b",  ".string","path=.string");
ENDLOADFUNCS

