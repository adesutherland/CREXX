/* REXX Program to process unicode database file (UnicopdeData.txt) to generate C/re2c normalisation routines */
/* Written by Adrian Sutherland August 2023 */

/* Config */
sort_queue_size = 100; /* Max number of non-starter codepoints that can be queued for sorting */

/* Read DerivedNormalizationProps.txt for Full_Composition_Exclusion, e.g.
    0340..0341    ; Full_Composition_Exclusion # Mn   [2] COMBINING GRAVE TONE MARK..COMBINING ACUTE TONE MARK
*/
derived_file = "DerivedNormalizationProps.txt"
full_composition_exclusion. = 0
do while lines(derived_file)
    line = linein(derived_file)
    if line = "" then iterate
    parse var line codes ";" type .
    if type \= "Full_Composition_Exclusion" then iterate
    parse var codes code1 ".." code2
    code1 = strip(code1)
    code2 = strip(code2)
    if code2 = "" then full_composition_exclusion.code1 = 1
    else do i = x2d(code1) to x2d(code2)
        code_point = d2x(i)
        if length(code_point) < 4 then code_point = right(code_point,4,"0")
        full_composition_exclusion.code_point = 1
    end
end
/* Close DerivedNormalizationProps.txt */
call lineout derived_file

/* Open UnicodeData.txt and loop though lines */
in_file = "UnicodeData.txt"
out_file = "norm_unicode.re"
'rm -f' out_file

codelist.0 = 0
valid_code_point. = 0
name. = ""
general_category. = ""
canonical_combining_class. = 0
bidi_class. = ""
compatibility_decomposition. = ""
canonical_decomposition. = ""
numeric_type. = ""
numeric_value. = ""
bidi_mirrored. = ""
unicode_1_name. = ""
iso_comment. = ""
simple_uppercase_mapping. = ""
simple_lowercase_mapping. = ""
simple_titlecase_mapping. = ""
processed_canonical_decomposition. = ""
processed_compatibility_decomposition. = ""
processed_canonical_recomposition. = ""
processed_compatibility_recomposition. = ""
canonical_max_decomposition = 0
compatibility_max_decomposition = 0
canonical_max_recomposition = 0
compatibility_max_recomposition = 0
inert. = 1
canonical_compos.0 = 0
canonical_compos_ix. = ""
type_count. = 0 /* Holds the number of codepoints of a particular type so we can count the impact of our algorithm */

/* Read Database file */
do while lines(in_file)
    line = linein(in_file)
    parse var line code_point ";" name ";" general_category ";" canonical_combining_class ";" bidi_class ";" decomposition_type ";" numeric_type ";" numeric_value ";" bidi_mirrored ";" unicode_1_name ";" iso_comment ";" simple_uppercase_mapping ";" simple_lowercase_mapping ";" simple_titlecase_mapping
    if code_point = "" then iterate

    if name = "<Hangul Syllable, First>" then do
       /* Algorithmically add all Hangul syllables */
       han_start = code_point
       line = linein(in_file)
       parse var line code_point ";" name ";" general_category ";" canonical_combining_class ";" bidi_class ";" decomposition_type ";" numeric_type ";" numeric_value ";" bidi_mirrored ";" unicode_1_name ";" iso_comment ";" simple_uppercase_mapping ";" simple_lowercase_mapping ";" simple_titlecase_mapping
       han_end = code_point

       do i = x2d(han_start) to x2d(han_end)
            code_point = d2x(i)
            c = codelist.0 + 1
            codelist.c = code_point
            codelist.0 = c
            valid_code_point.code_point = 1
            name.code_point = "<Hangul Syllable>"
            general_category.code_point = "Lo"
            canonical_combining_class.code_point = 0
            bidi_class.code_point = "L"
            canonical_decomposition.code_point = DecomposeHangul(code_point)
            compatibility_decomposition.code_point = canonical_decomposition.code_point
            numeric_type.code_point = ""
            numeric_value.code_point = ""
            bidi_mirrored.code_point = ""
            unicode_1_name.code_point = ""
            iso_comment.code_point = ""
            simple_uppercase_mapping.code_point = ""
            simple_lowercase_mapping.code_point = ""
            simple_titlecase_mapping.code_point = ""
      end
      iterate
    end

    /* Otherwise (non-Hangul) process record */
    c = codelist.0 + 1
    codelist.c = code_point
    codelist.0 = c

    /* Process decomposition_type */
    compatibility_decomposition = ""
    canonical_decomposition = ""
    if (left(decomposition_type,1) = "<") then do
        parse var decomposition_type  ">" compatibility_decomposition
        compatibility_decomposition = space(compatibility_decomposition)
    end
    else do
        canonical_decomposition = space(decomposition_type)
        compatibility_decomposition = canonical_decomposition
    end

    /* Store record */
    valid_code_point.code_point = 1
    name.code_point = name
    general_category.code_point = general_category
    canonical_combining_class.code_point = canonical_combining_class
    bidi_class.code_point = bidi_class
    compatibility_decomposition.code_point = compatibility_decomposition
    canonical_decomposition.code_point = canonical_decomposition
    numeric_type.code_point = numeric_type
    numeric_value.code_point = numeric_value
    bidi_mirrored.code_point = bidi_mirrored
    unicode_1_name.code_point = unicode_1_name
    iso_comment.code_point = iso_comment
    simple_uppercase_mapping.code_point = simple_uppercase_mapping
    simple_lowercase_mapping.code_point = simple_lowercase_mapping
    simple_titlecase_mapping.code_point = simple_titlecase_mapping
end
call lineout in_file

/* "Recursively" process canonical_decomposition and compatibility_decomposition */
do c = 1 to codelist.0
    code_point = codelist.c
    if valid_code_point.code_point = 0 then iterate
    call canonical_decompos code_point
    call compatibility_decompos code_point

    /* What is the max canonical decomposition length? */
    d = words(processed_canonical_decomposition.code_point)
    if d > canonical_max_decomposition then canonical_max_decomposition = d

    /* What is the max compatibility decomposition length? */
    d = words(processed_compatibility_decomposition.code_point)
    if d > compatibility_max_decomposition then compatibility_max_decomposition = d
end

/*
   Calculate the inert codepoints - these are the codepoints that are not decomposed in primary composites.
   Primary composite: A Canonical Decomposable Character (A character that is not identical to its canonical
   decomposition) which is not a Full Composition Exclusion.
   The list of primary composites can be computed by extracting all canonical decomposable characters and subtracting
   the list of Full Composition Exclusions.
*/
do i = 1 to codelist.0
    code_point = codelist.i
    if full_composition_exclusion.code_point then do
        iterate
    end
    decomp = space(canonical_decomposition.code_point) /* Decomposition of code_point */
    if decomp = "" then iterate
    if decomp = code_point then iterate

    do j = 1 to words(decomp)
        cp = word(decomp, j)
        inert.cp = 0
    end

    x = canonical_compos.0 + 1; canonical_compos.0 = x; canonical_compos.x = decomp
    canonical_compos_ix.decomp = code_point
end

/* Calculate the re-composition of the codepoints */
do i = 1 to codelist.0
    code_point = codelist.i

    /* Recompose canonical decompositions */
    decomposition_code_point = processed_canonical_decomposition.code_point
    if decomposition_code_point \= code_point then do
        /* This function returns the canonical decomposition of a codepoint - the key point of
           this algorithm is that this is valid because the starter is taken into account in the composition */
        processed_canonical_recomposition.code_point = canonical_compose(decomposition_code_point)
    end
    else processed_canonical_recomposition.code_point = code_point

    /* Recompose compatibility decompositions */
    decomposition_code_point = processed_compatibility_decomposition.code_point
    if decomposition_code_point \= code_point then do
        /* This function returns the canonical decomposition of a codepoint - the key point of
           this algorithm is that this is valid because the starter is taken into account in the composition */
        processed_compatibility_recomposition.code_point = canonical_compose(decomposition_code_point)
    end
    else processed_compatibility_recomposition.code_point = code_point

    /* What is the max canonical recomposition length? */
    d = words(processed_canonical_recomposition.code_point)
    if d > canonical_max_recomposition then canonical_max_recomposition = d

    /* What is the max compatibility recomposition length? */
    d = words(processed_compatibility_recomposition.code_point)
    if d > compatibility_max_recomposition then compatibility_max_recomposition = d
end

/* Make character classes for characters with no canonical decomposition by canonical_combining_class */
canonical_ccc_class. = ""
start_code = ""
end_code = ""
last_c = ""
do i = 1 to codelist.0
    code_point = codelist.i
    c = canonical_combining_class.code_point;
    if last_c = "" then last_c = c

    /* Write out range if c changes or the code_point is decomposed */
    if last_c \= c | "["processed_canonical_decomposition.code_point"]" \= "["code_point"]" then do
        /* Write out range */
        if start_code \= "" then do
            if "["start_code"]" = "["end_code"]" then
                canonical_ccc_class.last_c = canonical_ccc_class.last_c || escape(start_code, length(start_code))
            else do
                len = max(length(start_code),length(end_code))
                canonical_ccc_class.last_c = canonical_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
            end
        end
        start_code = ""
        end_code = ""
        last_c = c
    end

    /* Add to range if code_point is not decomposed */
    if "["processed_canonical_decomposition.code_point"]" = "["code_point"]"  then do
        if start_code = "" then start_code = code_point
        end_code = code_point
        if c = 0 then type_count._NO_CAN_DECOM_STARTER = type_count._NO_CAN_DECOM_STARTER + 1
        else type_count._NO_CAN_DECOM_NON_STARTER = type_count._NO_CAN_DECOM_NON_STARTER + 1
    end
end

/* Process last range */
if start_code > 0 then do
    if "["start_code"]" = "["end_code"]" then
        canonical_ccc_class.last_c = canonical_ccc_class.last_c || escape(start_code, length(start_code))
    else do
        len = max(length(start_code),length(end_code))
        canonical_ccc_class.last_c = canonical_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
    end
end

/* finalise canonical_ccc_class.c */
do c = 0 to 255
    if canonical_ccc_class.c \= "" then canonical_ccc_class.c = "no_canonical_decom_ccc_"c "= [" || canonical_ccc_class.c || "];"
end

/* Make character classes for characters with no compatibility decomposition by canonical_combining_class */
compatibility_ccc_class. = ""
start_code = ""
end_code = ""
last_c = ""
do i = 1 to codelist.0
    code_point = codelist.i
    c = canonical_combining_class.code_point;
    if last_c = "" then last_c = c

    /* Write out range if c changes or the code_point is decomposed */
    if last_c \= c | "["processed_compatibility_decomposition.code_point"]" \= "["code_point"]" then do
        /* Write out range */
        if start_code \= "" then do
            if "["start_code"]" = "["end_code"]" then
                compatibility_ccc_class.last_c = compatibility_ccc_class.last_c || escape(start_code, length(start_code))
            else do
                len = max(length(start_code),length(end_code))
                compatibility_ccc_class.last_c = compatibility_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
            end
        end
        start_code = ""
        end_code = ""
        last_c = c
    end

    /* Add to range if code_point is not decomposed */
    if "["processed_compatibility_decomposition.code_point"]" = "["code_point"]" then do
        if start_code = "" then start_code = code_point
        end_code = code_point
        if c = 0 then type_count._NO_COM_DECOM_STARTER = type_count._NO_COM_DECOM_STARTER + 1
        else type_count._NO_COM_DECOM_NON_STARTER = type_count._NO_COM_DECOM_NON_STARTER + 1
    end
end

/* Process last range */
if start_code > 0 then do
    if "["start_code"]" = "["end_code"]" then
        compatibility_ccc_class.last_c = compatibility_ccc_class.last_c || escape(start_code, length(start_code))
    else do
        len = max(length(start_code),length(end_code))
        compatibility_ccc_class.last_c = compatibility_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
    end
end

/* finalise compatibility_ccc_class.c */
do c = 0 to 255
    if compatibility_ccc_class.c \= "" then compatibility_ccc_class.c = "no_compatibility_decom_ccc_"c "= [" || compatibility_ccc_class.c || "];"
end

/* Make character classes for characters with no canonical recomposition for inert codepoints by canonical_combining_class */
inert_ccc_class. = ""
start_code = ""
end_code = ""
last_c = ""
do i = 1 to codelist.0
    code_point = codelist.i
    c = canonical_combining_class.code_point;
    if last_c = "" then last_c = c

    /* Write out range if c changes, the code_point is decomposed or the code_point is not inert */
    if last_c \= c | "["processed_canonical_recomposition.code_point"]" \= "["code_point"]" | inert.code_point = 0 then do
        /* Write out range */
        if start_code \= "" then do
            if "["start_code"]" = "["end_code"]" then
                inert_ccc_class.last_c = inert_ccc_class.last_c || escape(start_code, length(start_code))
            else do
                len = max(length(start_code),length(end_code))
                inert_ccc_class.last_c = inert_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
            end
        end
        start_code = ""
        end_code = ""
        last_c = c
    end

    /* Add to range if code_point is not recomposed */
    if "["processed_canonical_recomposition.code_point"]" = "["code_point"]" & inert.code_point = 1 then do



        /* Check if the intermediate codepoint is inert */
        intermediate_code_point = processed_canonical_decomposition.code_point
        if inert.intermediate_code_point = 0  then say "Error: Codepoint" code_point "intermediate" intermediate_code_point "is not inert"




        if start_code = "" then start_code = code_point
        end_code = code_point
        if c = 0 then type_count._NO_CAN_RECOM_INERT_STARTER = type_count._NO_CAN_RECOM_INERT_STARTER + 1
        else type_count._NO_CAN_RECOM_INERT_NON_STARTER = type_count._NO_CAN_RECOM_INERT_NON_STARTER + 1
    end
end

/* Process last range */
if start_code > 0 then do
    if "["start_code"]" = "["end_code"]" then
        inert_ccc_class.last_c = inert_ccc_class.last_c || escape(start_code, length(start_code))
    else do
        len = max(length(start_code),length(end_code))
        inert_ccc_class.last_c = inert_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
    end
end

/* finalise inert_ccc_class.c */
do c = 0 to 255
    if inert_ccc_class.c \= "" then inert_ccc_class.c = "inert_ccc_"c "= [" || inert_ccc_class.c || "];"
end

/* Make character classes for characters with no canonical recomposition for non-inert codepoints by canonical_combining_class */
non_inert_ccc_class. = ""
start_code = ""
end_code = ""
last_c = ""
do i = 1 to codelist.0
    code_point = codelist.i
    c = canonical_combining_class.code_point;
    if last_c = "" then last_c = c

    /* Write out range if c changes, the code_point is decomposed or the code_point is not inert */
    if last_c \= c | "["processed_canonical_recomposition.code_point"]" \= "["code_point"]" | inert.code_point = 1 then do
        /* Write out range */
        if start_code \= "" then do
            if "["start_code"]" = "["end_code"]" then
                non_inert_ccc_class.last_c = non_inert_ccc_class.last_c || escape(start_code, length(start_code))
            else do
                len = max(length(start_code),length(end_code))
                non_inert_ccc_class.last_c = non_inert_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
            end
        end
        start_code = ""
        end_code = ""
        last_c = c
    end

    /* Add to range if code_point is not decomposed */
    if "["processed_canonical_recomposition.code_point"]" = "["code_point"]" & inert.code_point = 0 then do
        if start_code = "" then start_code = code_point
        end_code = code_point
        if c = 0 then type_count._NO_CAN_RECOM_NON_INERT_STARTER = type_count._NO_CAN_RECOM_NON_INERT_STARTER + 1
        else type_count._NO_CAN_RECOM_NON_INERT_NON_STARTER = type_count._NO_CAN_RECOM_NON_INERT_NON_STARTER + 1

    end
end

/* Process last range */
if start_code > 0 then do
    if "["start_code"]" = "["end_code"]" then
        non_inert_ccc_class.last_c = non_inert_ccc_class.last_c || escape(start_code, length(start_code))
    else do
        len = max(length(start_code),length(end_code))
        non_inert_ccc_class.last_c = non_inert_ccc_class.last_c || escape(start_code, len)"-"escape(end_code, len)
    end
end

/* finalise non_inert_ccc_class.c */
do c = 0 to 255
    if non_inert_ccc_class.c \= "" then non_inert_ccc_class.c = "non_inert_ccc_"c "= [" || non_inert_ccc_class.c || "];"
end

/*
   Blocked:Let A and C be two characters in a coded character sequence <A,...C>.C is blocked from A if and only
   if ccc(A) = 0 and there exists some character B between A and C in the coded character sequence,
   i.e., <A, ... B, ... C>, and either ccc(B) = 0 or ccc(B) >= ccc(C).

   Canonical Composition Algorithm: Starting from the second character in the coded character sequence (of a Canonical
   Decomposition or Compatibility Decomposition) and proceeding sequentially to the final character, perform the following
   steps:

     Seek back (left) in the coded character sequence from the character C to find the last Starter L preceding C in the
     character sequence.

     If there is such an L, and C is not blocked from L, and there exists a Primary Composite P which is canonically
     equivalent to the sequence <L, C>, then replace L by P in the sequence and delete C from the sequence.

   When the algorithm completes, all Non-blocked Pairs canonically equivalent to a Primary Composite will have been
   systematically replaced by those Primary Composites.
*/


/*
/* REXX script for Unicode Normalisation Test Cases (Part 0) */

/* Test 1 */
nfd = "0044 0307"
expected_nfc = "1E0A"
call testNormalization nfd, expected_nfc

/* Test 2 */
nfd = "0044 0323"
expected_nfc = "1E0C"
call testNormalization nfd, expected_nfc

/* Test 3 */
nfd = "0044 0323 0307"
expected_nfc = "1E0C 0307"
call testNormalization nfd, expected_nfc

/* Test 4 */
nfd = "0044 031B 0307"
expected_nfc = "1E0A 031B"
call testNormalization nfd, expected_nfc

/* Test 5 */
nfd = "0044 031B 0323 0307"
expected_nfc = "1E0C 031B 0307"
call testNormalization nfd, expected_nfc

/* Test 6 */
nfd = "0045 0300"
expected_nfc = "00C8"
call testNormalization nfd, expected_nfc

/* Test 7 */
nfd = "0045 0304 0300"
expected_nfc = "1E14"
call testNormalization nfd, expected_nfc

/* Test 8 */
nfd = "1100 1100 1161 11A8"
expected_nfc = "1100 AC01"
call testNormalization nfd, expected_nfc

/* Test 9 */
call testNormalization "1100 1100 1161 11A8 11A8", "1100 AC01 11A8"

/* Test 10 */
call testNormalization "1100 1161 11A8 11A8", "AC01 11A8"

/* Test 11 */
call testNormalization "1100 1161 11A8 11A8 11A8", "AC01 11A8 11A8"

/* Test 12 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8"

/* Test 13 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8"

/* Test 14 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8"

/* Test 15 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8 11A8"

/* Test 16 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8 11A8 11A8"

/* Test 17 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8"

/* Test 18 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8"

/* Test 19 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8"

/* Test 20 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8"

/* Test 21 */
call testNormalization "1100 1161 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8", "AC01 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8 11A8"

/* Test function */
testNormalization:
    parse arg nfd, expected_nfc
    actual_nfc = canonical_compose(nfd)
    if actual_nfc \= expected_nfc then do
        say "Error: Test case failed. Expected:" expected_nfc ", Got:" actual_nfc
        exit
    end
    return
*/



/* Generate file */
/* Write the head of the out_file */
call lineout out_file, "/* Unicode Normaliser - generated by processunicodedata.rexx baseed on unicode database file (UnicopdeData.txt) */"
call lineout out_file, "/* processunicodedata.rexx written by Adrian Sutherland August 2023 */"
call lineout out_file, "/* This file was generated at" date() time() "*/"
call lineout out_file, "#include <stdio.h>"
call lineout out_file, "#include <string.h>"
call lineout out_file, "#include <stdlib.h>"
call lineout out_file, ""
call lineout out_file, "#define SORT_QUEUE_SIZE" sort_queue_size
call lineout out_file, ""
call lineout out_file, "/*!re2c"
call lineout out_file, "   any = [^];"
call lineout out_file, "*/"
call lineout out_file, ""
call lineout out_file, "/*!re2c"
call lineout out_file, "    re2c:define:YYCTYPE = 'unsigned char';"
call lineout out_file, "    re2c:yyfill:enable = 0;"
call lineout out_file, "    re2c:define:YYCURSOR  = str;"
call lineout out_file, "    re2c:encoding:utf8 = 1;"
call lineout out_file, "    re2c:eof = 0;"
call lineout out_file, "*/"
call lineout out_file, ""

/* Write out NFD Normaliser */
call lineout out_file, "int nfd_normaliser(const char *input, int len, char* out, int *out_len) {"
call lineout out_file, "    const unsigned char *str = (const unsigned char *)input;"
call lineout out_file, "    const unsigned char *start;"
call lineout out_file, "    const unsigned char *YYLIMIT = str + len, *YYMARKER, *YYCTXMARKER;"
call lineout out_file, "    const char* single_cp;"
call lineout out_file, "    int single_cp_len;"
call lineout out_file, "    int single_ccc;"
call lineout out_file, "    int i, j, k;"
call lineout out_file, "    char* multiple_cp["canonical_max_decomposition"];"
call lineout out_file, "    int multiple_cp_len["canonical_max_decomposition"];"
call lineout out_file, "    int multiple_ccc["canonical_max_decomposition"];"
call lineout out_file, "    int multiple_cp_count = 0;"
call lineout out_file, "    const char* queued_cp[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cp_len[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cp_ccc[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cps = 0;"
call lineout out_file, ""
call lineout out_file, "    *out_len = 0;"
call lineout out_file, "    /* Loop for whole string */"
call lineout out_file, "    for (;;) {"
call lineout out_file, "        start = str;"
call lineout out_file, ""
call lineout out_file, ""
call lineout out_file, "/*!re2c"
call lineout out_file, ""

/* write out classes with not decomposition */
call lineout out_file, "        // classes with not decomposition by ccc"
do cc = 0 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if canonical_ccc_class.cc \= "" then do
        call lineout out_file, '       ' canonical_ccc_class.cc
    end
end
call lineout out_file, ""

call lineout out_file, "        // Dispatch classes with not decomposition by ccc"
if canonical_ccc_class.0 \= "" then do
    call lineout out_file, '        no_canonical_decom_ccc_'0 '{single_cp = (char*)start; single_cp_len = (int)(str - start); goto starter;}'
end
do cc = 1 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if canonical_ccc_class.cc \= "" then do
        call lineout out_file, '        no_canonical_decom_ccc_'cc '{single_ccc =' cc'; single_cp = (char*)start; single_cp_len = (int)(str - start); goto single;}'
    end
end
call lineout out_file, ""

/* Write out individul unicode decompositions where the unicode is decomposed */
call lineout out_file, "        // Dispatch unicode with decomposition"
do c = 1 to codelist.0
    code_point = codelist.c
    if valid_code_point.code_point = 0 then iterate
    if processed_canonical_decomposition.code_point \= code_point then do
        func_args = words(processed_canonical_decomposition.code_point)
        cd = word(processed_canonical_decomposition.code_point, 1)
        last_cd = word(processed_canonical_decomposition.code_point, func_args)
        if func_args = 1 then do
            /* Single codepoint - goto starter: or single: */
            if canonical_combining_class.cd = 0 then do
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8(cd)'; single_cp_len =' utf8_bytes(cd)'; goto starter;}'
                type_count._CAN_DECOM_STARTER = type_count._CAN_DECOM_STARTER + 1
            end
            else do
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_ccc =' canonical_combining_class.cd'; single_cp = (char*)'to_utf8(cd)'; single_cp_len =' utf8_bytes(cd)'; goto single;}'
                type_count._CAN_DECOM_SINGLE = type_count._CAN_DECOM_SINGLE + 1
            end
        end
        else if canonical_combining_class.cd = 0 & canonical_combining_class.last_cd = 0 then do
            /* Multiple codepoints start and ending with ccc = 0 - goto starter: */
            call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8_string(processed_canonical_decomposition.code_point)'; single_cp_len =' utf8_string_bytes(processed_canonical_decomposition.code_point)'; goto starter;}'
            type_count._CAN_DECOM_STARTER = type_count._CAN_DECOM_STARTER + 1
        end
        else do
            /* See if the first codepoints are starters, followed by non-starters only, and record the position of the last starter */
            starter_followed_by_non_starters = 1
            last_starter = 0
            if canonical_combining_class.cd = 0 then do
                do w = 2 to words(processed_canonical_decomposition.code_point)
                    cp = word(processed_canonical_decomposition.code_point, w)
                    if canonical_combining_class.cp = 0 then do
                        if last_starter =\ 0 then do
                            starter_followed_by_non_starters = 0
                            leave
                        end
                    end
                    else if last_starter = 0 then last_starter = w - 1
                end
            end
            else starter_followed_by_non_starters = 0

            if starter_followed_by_non_starters then do
                /* load multiple_cp[] etc. with the non-starters */
                mult = "multiple_cp_count =" (words(processed_canonical_decomposition.code_point) - last_starter)"; "
                ix = 0
                do w = last_starter + 1 to words(processed_canonical_decomposition.code_point)
                    cp = word(processed_canonical_decomposition.code_point, w)
                    ccc = canonical_combining_class.cp
                    mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc";"
                    ix = ix + 1
                end

                /* Branch to starter_then_non_starter: */
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8_string(subword(processed_canonical_decomposition.code_point,1,last_starter)) ||,
                    '; single_cp_len =' utf8_string_bytes(subword(processed_canonical_decomposition.code_point,1,last_starter)) ||,
                    ';' mult 'goto starter_then_non_starter;}'
                type_count._CAN_DECOM_STARTER_THEN_NON_STARTER = type_count._CAN_DECOM_STARTER_THEN_NON_STARTER + 1
            end

            else do
                /* load multiple_cp[] etc. */
                mult = "multiple_cp_count =" (words(processed_canonical_decomposition.code_point))"; "
                ix = 0
                do w = 1 to words(processed_canonical_decomposition.code_point)
                    cp = word(processed_canonical_decomposition.code_point, w)
                    ccc = canonical_combining_class.cp
                    mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc";"
                    ix = ix + 1
                end

                /* Branch to complex: */
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {' mult 'goto complex;}'
                type_count._CAN_DECOM_COMPLEX = type_count._CAN_DECOM_COMPLEX + 1
            end
        end
    end
end

/* Write the tail of the normaliser */
call write_nfkd_and_nfd_normaliser_tail

/* Write out NFKD Normaliser */
call lineout out_file, "int nfkd_normaliser(const char *input, int len, char* out, int *out_len) {"
call lineout out_file, "    const unsigned char *str = (const unsigned char *)input;"
call lineout out_file, "    const unsigned char *start;"
call lineout out_file, "    const unsigned char *YYLIMIT = str + len, *YYMARKER, *YYCTXMARKER;"
call lineout out_file, "    const char* single_cp;"
call lineout out_file, "    int single_cp_len;"
call lineout out_file, "    int single_ccc;"
call lineout out_file, "    int i, j, k;"
call lineout out_file, "    char* multiple_cp["compatibility_max_decomposition"];"
call lineout out_file, "    int multiple_cp_len["compatibility_max_decomposition"];"
call lineout out_file, "    int multiple_ccc["compatibility_max_decomposition"];"
call lineout out_file, "    int multiple_cp_count = 0;"
call lineout out_file, "    const char* queued_cp[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cp_len[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cp_ccc[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cps = 0;"
call lineout out_file, ""
call lineout out_file, "    *out_len = 0;"
call lineout out_file, "    /* Loop for whole string */"
call lineout out_file, "    for (;;) {"
call lineout out_file, "        start = str;"
call lineout out_file, ""
call lineout out_file, ""
call lineout out_file, "/*!re2c"
call lineout out_file, ""

/* write out classes with no decomposition */
call lineout out_file, "        // classes with not decomposition by ccc"
do cc = 0 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if compatibility_ccc_class.cc \= "" then do
        call lineout out_file, '       ' compatibility_ccc_class.cc
    end
end
call lineout out_file, ""

call lineout out_file, "        // Dispatch classes with not decomposition by ccc"
if compatibility_ccc_class.0 \= "" then do
    call lineout out_file, '        no_compatibility_decom_ccc_'0 '{single_cp = (char*)start; single_cp_len = (int)(str - start); goto starter;}'
end
do cc = 1 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if compatibility_ccc_class.cc \= "" then do
        call lineout out_file, '        no_compatibility_decom_ccc_'cc '{single_ccc =' cc'; single_cp = (char*)start; single_cp_len = (int)(str - start); goto single;}'
    end
end
call lineout out_file, ""

/* Write out individul unicode decompositions where the unicode is decomposed */
call lineout out_file, "        // Dispatch unicode with decomposition"
do c = 1 to codelist.0
    code_point = codelist.c
    if valid_code_point.code_point = 0 then iterate
    if processed_compatibility_decomposition.code_point \= code_point then do
        func_args = words(processed_compatibility_decomposition.code_point)
        cd = word(processed_compatibility_decomposition.code_point, 1)
        last_cd = word(processed_compatibility_decomposition.code_point, func_args)
        if func_args = 1 then do
            /* Single codepoint - goto starter: or single: */
            if canonical_combining_class.cd = 0 then do
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8(cd)'; single_cp_len =' utf8_bytes(cd)'; goto starter;}'
                type_count._COM_DECOM_STARTER = type_count._COM_DECOM_STARTER + 1
            end
            else do
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_ccc =' canonical_combining_class.cd'; single_cp =(char*)'to_utf8(cd)'; single_cp_len =' utf8_bytes(cd)'; goto single;}'
                type_count._COM_DECOM_SINGLE = type_count._COM_DECOM_SINGLE + 1
            end
        end
        else if canonical_combining_class.cd = 0 & canonical_combining_class.last_cd = 0 then do
            /* Multiple codepoints start and ending with ccc = 0 - goto starter: */
            call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8_string(processed_compatibility_decomposition.code_point)'; single_cp_len =' utf8_string_bytes(processed_compatibility_decomposition.code_point)'; goto starter;}'
            type_count._COM_DECOM_STARTER = type_count._COM_DECOM_STARTER + 1
        end
        else do
            /* See if the first codepoints are starters, followed by non-starters only, and record the position of the last starter */
            starter_followed_by_non_starters = 1
            last_starter = 0
            if canonical_combining_class.cd = 0 then do
                do w = 2 to words(processed_compatibility_decomposition.code_point)
                    cp = word(processed_compatibility_decomposition.code_point, w)
                    if canonical_combining_class.cp = 0 then do
                        if last_starter =\ 0 then do
                            starter_followed_by_non_starters = 0
                            leave
                        end
                    end
                    else if last_starter = 0 then last_starter = w - 1
                end
            end
            else starter_followed_by_non_starters = 0

            if starter_followed_by_non_starters then do
                /* load multiple_cp[] etc. with the non-starters */
                mult = "multiple_cp_count =" (words(processed_compatibility_decomposition.code_point) - last_starter)"; "
                ix = 0
                do w = last_starter + 1 to words(processed_compatibility_decomposition.code_point)
                    cp = word(processed_compatibility_decomposition.code_point, w)
                    ccc = canonical_combining_class.cp
                    mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc";"
                    ix = ix + 1
                end

                /* Branch to starter_then_non_starter: */
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8_string(subword(processed_compatibility_decomposition.code_point,1,last_starter)) ||,
                    '; single_cp_len =' utf8_string_bytes(subword(processed_compatibility_decomposition.code_point,1,last_starter)) ||,
                    ';' mult 'goto starter_then_non_starter;}'
                type_count._COM_DECOM_STARTER_THEN_NON_STARTER = type_count._COM_DECOM_STARTER_THEN_NON_STARTER + 1
            end

            else do
                /* load multiple_cp[] etc. */
                mult = "multiple_cp_count =" (words(processed_compatibility_decomposition.code_point))"; "
                ix = 0
                do w = 1 to words(processed_compatibility_decomposition.code_point)
                    cp = word(processed_compatibility_decomposition.code_point, w)
                    ccc = canonical_combining_class.cp
                    mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc";"
                    ix = ix + 1
                end

                /* Branch to complex: */
                call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {' mult 'goto complex;}'
                type_count._COM_DECOM_COMPLEX = type_count._COM_DECOM_COMPLEX + 1
            end
        end
    end
end

/* Write the tail of the normaliser */
call write_nfkd_and_nfd_normaliser_tail

/* Write out Canonical Composer */
call lineout out_file, "/* Internal function to canonical compose two codepoints (cp1 and cp2 in utf8 format) */"
call lineout out_file, "static char* canonical_composer(const char *cp1, const char *cp2, int *out_len, int *inert, int *ccc) {"
call lineout out_file, "    const unsigned char *YYLIMIT, *YYMARKER, *YYCTXMARKER;"
call lineout out_file, "    int i, char_len;"
call lineout out_file, "    unsigned char buffer[10];"
call lineout out_file, "    const unsigned char *str = buffer;"
call lineout out_file, "    unsigned char *s = buffer;"
call lineout out_file, "    unsigned char *byte;"
call lineout out_file, ""
call lineout out_file  "    /* Copy cp1 and cp2 using utf8 length semantics (not assuming null termination) */"
call lineout out_file, "    /* First do cp1 */"
call lineout out_file, "    byte = (unsigned char*)cp1;"
call lineout out_file, "    if (*byte < 0x80) char_len = 1;"
call lineout out_file, "    else if (*byte < 0xE0) char_len = 2;"
call lineout out_file, "    else if (*byte < 0xF0) char_len = 3;"
call lineout out_file, "    else char_len = 4;"
call lineout out_file, "    switch (char_len) {"
call lineout out_file, "        case 4: *s++ = *byte++;"
call lineout out_file, "        case 3: *s++ = *byte++;"
call lineout out_file, "        case 2: *s++ = *byte++;"
call lineout out_file, "        case 1: *s++ = *byte++;"
call lineout out_file, "    }"
call lineout out_file, ""
call lineout out_file, "    /* Now do cp2 */"
call lineout out_file, "    byte = (unsigned char*)cp2;"
call lineout out_file, "    if (*byte < 0x80) char_len = 1;"
call lineout out_file, "    else if (*byte < 0xE0) char_len = 2;"
call lineout out_file, "    else if (*byte < 0xF0) char_len = 3;"
call lineout out_file, "    else char_len = 4;"
call lineout out_file, "    switch (char_len) {"
call lineout out_file, "        case 4: *s++ = *byte++;"
call lineout out_file, "        case 3: *s++ = *byte++;"
call lineout out_file, "        case 2: *s++ = *byte++;"
call lineout out_file, "        case 1: *s++ = *byte++;"
call lineout out_file, "    }"
call lineout out_file, "    *s = 0;"
call lineout out_file, ""
call lineout out_file, "    YYLIMIT = s;"
call lineout out_file, '    printf("length of buffer is %d\n", (int)(YYLIMIT - str));'
call lineout out_file, "    *out_len = 0;"
call lineout out_file, ""
call lineout out_file, "/*!re2c"

/* Write out individual unicode compositions */
call lineout out_file, "    // Dispatch unicode with composition"
do c = 1 to canonical_compos.0
    decomposed_code_point = canonical_compos.c
    code_point = canonical_compos_ix.decomposed_code_point
    inert = inert.code_point
    ccc = canonical_combining_class.code_point

    decomp = words(decomposed_code_point)
    if decomp \= 2 then do
        say "ERROR:" code_point "has more than two codepoints in its decomposition"
        exit 1
    end
    find = '"'
    do x = 1 to decomp
       cp = word(decomposed_code_point, x)
       find = find || escape(cp, length(strip(cp,"L","0")))
    end
    find = find || '"'
    call lineout out_file, '   ' find '{ *out_len =' utf8_bytes(code_point)'; *inert =' inert'; *ccc =' ccc'; return' to_utf8(code_point)';} // -> U+'code_point
end
call lineout out_file, ""
call lineout out_file, "    // Everything else has no composition"
call lineout out_file, '    any {return 0;}'
call lineout out_file, ""
call lineout out_file, "    // Invalid stuff"
call lineout out_file, '    *  { printf("ERROR invalid input [%s]\n", buffer); }'
call lineout out_file, ""
call lineout out_file, "    // Unexpected End of Input"
call lineout out_file, '    $   { printf("ERROR Unexpected EOI [%s]\n", buffer); }'
call lineout out_file, "*/"
call lineout out_file, ""
call lineout out_file, "    return 0;"
call lineout out_file, "}"
call lineout out_file, ""

/* Write out NFC Normaliser */
call lineout out_file, "int nfc_normaliser(const char *input, int len, char* out, int *out_len) {"
call lineout out_file, "    const unsigned char *str = (const unsigned char *)input;"
call lineout out_file, "    const unsigned char *start;"
call lineout out_file, "    const unsigned char *YYLIMIT = str + len, *YYMARKER, *YYCTXMARKER;"
call lineout out_file, "    const char* single_cp = 0;"
call lineout out_file, "    int single_cp_len = 0;"
call lineout out_file, "    int single_ccc = 0;"
call lineout out_file, "    int single_inert = 0;"
call lineout out_file, "    int i, j, k;"
call lineout out_file, "    char* multiple_cp["canonical_max_recomposition"];"
call lineout out_file, "    int multiple_cp_len["canonical_max_recomposition"];"
call lineout out_file, "    int multiple_ccc["canonical_max_recomposition"];"
call lineout out_file, "    int multiple_inert["canonical_max_recomposition"];"
call lineout out_file, "    int multiple_cp_count = 0;"
call lineout out_file, "    const char* queued_cp[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cp_len[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cp_ccc[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cp_inert[SORT_QUEUE_SIZE];"
call lineout out_file, "    int queued_cps = 0;"
call lineout out_file, ""
call lineout out_file, "    *out_len = 0;"
call lineout out_file, "    /* Loop for whole string */"
call lineout out_file, "    for (;;) {"
call lineout out_file, "        start = str;"
call lineout out_file, ""
call lineout out_file, ""
call lineout out_file, "/*!re2c"
call lineout out_file, ""

/* write out inert classes with no recomposition */
call lineout out_file, "        // inert classes with no recomposition by ccc"
do cc = 0 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if inert_ccc_class.cc \= "" then do
        call lineout out_file, '       ' inert_ccc_class.cc
    end
end
call lineout out_file, ""

/* write out non-inert classes with no recomposition */
call lineout out_file, "        // non-inert classes with no recomposition by ccc"
do cc = 0 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if non_inert_ccc_class.cc \= "" then do
        call lineout out_file, '       ' non_inert_ccc_class.cc
    end
end
call lineout out_file, ""

/*
    These are the treatment classifications / goto targets (IS=Inert Starter S=Starter IX=Inert Single X=Single)
       TP1. IS -> inert_starter
       TP2. S -> starter
       TP3. IX -> inert_single
       TP5. IS (X|IX)+ IS -> inert_starter
       TP7. S (X|IX)+ (S|IS) -> multi_starter
       TP8. IS (IX)+ inert_starter_then_non_starter
       TP10. Otherwise -> full
*/

/* Dispatch inert classes with no recomposition by ccc */
call lineout out_file, "        // Dispatch inert classes with no recomposition by ccc"
if inert_ccc_class.0 \= "" then do
    call lineout out_file, '        inert_ccc_'0 '{single_cp = (char*)start; single_cp_len = (int)(str - start); goto inert_starter;} // TP1'
end
do cc = 1 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if inert_ccc_class.cc \= "" then do
        call lineout out_file, '        inert_ccc_'cc '{single_ccc =' cc'; single_cp = (char*)start; single_cp_len = (int)(str - start); goto inert_single;} // TP3'
    end
end
call lineout out_file, ""

/* Dispatch non-inert classes with no recomposition by ccc */
call lineout out_file, "        // Dispatch non-inert classes with no recomposition by ccc"
if non_inert_ccc_class.0 \= "" then do
    call lineout out_file, '        non_inert_ccc_'0 '{single_cp = (char*)start; single_cp_len = (int)(str - start); goto starter;} // TP2'
end
do cc = 1 to 255 /* Currently the highest canonical_combining_class is 254, 255 for future proofing a bit */
    if non_inert_ccc_class.cc \= "" then do
        /* full target only supports multiple codepoints, so we need to use multiple_cp[] etc */
        call lineout out_file, '        non_inert_ccc_'cc '{multiple_cp_count = 1; multiple_cp[0] = (char*)start; multiple_cp_len[0] = (int)(str - start); multiple_ccc[0] =' cc'; goto full;} // TP10'
    end
end
call lineout out_file, ""

/* Write out individual unicode recompositions where the unicode is decomposed */
call lineout out_file, "        // Dispatch unicode with recomposition"
do c = 1 to codelist.0
    code_point = codelist.c
    if valid_code_point.code_point = 0 then iterate

    canonical_composed = processed_canonical_recomposition.code_point
    if canonical_composed \= code_point then do /* Codepoints not decomposed were handled in the classes above */
        num_code_points = words(canonical_composed)
        if num_code_points = 1 then do
            /*
                Single codepoint:
                TP1. IS -> inert_starter
                TP2. S -> starter
                TP3. IX -> inert_single
                TP4. X -> full
            */
            ccc = canonical_combining_class.canonical_composed
            inert = inert.canonical_composed
            select
                when ccc = 0 & inert = 1 then do
                    call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8(canonical_composed)'; single_cp_len =' utf8_bytes(canonical_composed)'; goto inert_starter;} // TP1' code_point'->'canonical_composed
                    type_count._CAN_RECOM_INERT_STARTER = type_count._CAN_RECOM_INERT_STARTER + 1
                end
                when ccc = 0 then do
                    call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8(canonical_composed)'; single_cp_len =' utf8_bytes(canonical_composed)'; goto starter;} // TP2' code_point'->'canonical_composed
                    type_count._CAN_RECOM_STARTER = type_count._CAN_RECOM_STARTER + 1
                end
                when inert = 1 then do /* ccc > 0 */
                    call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_ccc =' ccc'; single_cp = (char*)'to_utf8(canonical_composed)'; single_cp_len =' utf8_bytes(canonical_composed)'; goto inert_single;}  // TP3' code_point'->'canonical_composed
                    type_count._CAN_RECOM_INERT_SINGLE = type_count._CAN_RECOM_INERT_SINGLE + 1
                end
                otherwise do /* ccc > 0 and inert = 0 */
                    /* full target only supports multiple codepoints, so we need to use multiple_cp[] etc */
                    call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {multiple_cp_count = 1; multiple_cp[0] = (char*)'to_utf8(canonical_composed)'; multiple_cp_len[0] =' utf8_bytes(canonical_composed)'; multiple_ccc[0] =' ccc'; goto full;} // TP10' code_point'->'canonical_composed
                    type_count._COM_DECOM_COMPLEX = type_count._COM_DECOM_COMPLEX + 1
                end
            end
        end

        else do
            cd = word(canonical_composed, 1)
            last_cd = word(canonical_composed, num_code_points)
            if canonical_combining_class.cd = 0 & canonical_combining_class.last_cd = 0 then do
                /* Multiple codepoints start and ending with ccc = 0
                   5. IS (X|IX)+ IS -> inert_starter
                   7. (S|IS) (X|IX)+ (S|IS) -> multi_starter
                */
                if inert.cd & inert.last_cd then do
                    /* 5. IS (X|IX)+ IS -> inert_starter */
                    call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8_string(canonical_composed)'; single_cp_len =' utf8_string_bytes(canonical_composed)'; goto inert_starter;} // TP5' code_point'->'canonical_composed
                    type_count._CAN_RECOM_INERT_STARTER = type_count._CAN_RECOM_INERT_STARTER + 1
                end
                else do
                    /* 7. (S|IS) (X|IX)+ (S|IS) -> multi_starter */
                    /* load multiple_cp[] etc. */
                    mult = "multiple_cp_count =" (words(canonical_composed))"; "
                    ix = 0
                    do w = 1 to words(canonical_composed)
                        cp = word(canonical_composed, w)
                        ccc = canonical_combining_class.cp
                        inert = inert.cp
                        mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc"; multiple_inert["ix"] =" inert";"
                        ix = ix + 1
                    end

                    /* Branch */
                    call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {' mult 'goto multi_starter;} // TP7' code_point'->'canonical_composed
                    type_count._CAN_RECOM_MULTI_STARTER = type_count._CAN_RECOM_MULTI_STARTER + 1
                end
            end
            else do
                /* See if the first codepoints are starters, followed by non-starters only, and record the position of the last starter */
                /*
                       8. IS (IX)+ inert_starter_then_non_starter
                       9. (S|IS) (X|IX)+ full
                */
                starter_followed_by_non_starters = 1
                last_starter = 0
                if canonical_combining_class.cd = 0 then do
                    do w = 2 to words(canonical_composed)
                        cp = word(canonical_composed, w)
                        if canonical_combining_class.cp = 0 then do
                            if last_starter =\ 0 then do
                                starter_followed_by_non_starters = 0
                                leave
                            end
                        end
                        else if last_starter = 0 then last_starter = w - 1
                    end
                end
                else starter_followed_by_non_starters = 0

                if starter_followed_by_non_starters then do

                    /* inert? */
                    inert = 1
                    do w = 1 to words(canonical_composed) while inert
                        cp = word(canonical_composed, w)
                        if inert.cp = 0 then inert = 0
                    end

                    if inert = 0 then last_starter = 0 /* "goto full" target used in the non-inert scenario (see later) does not support single_cp must use multiple_cp[] for all codepoints */

                    /* load multiple_cp[] etc. with the non-starters */
                    mult = "multiple_cp_count =" (words(canonical_composed) - last_starter)"; "
                    ix = 0
                    do w = last_starter + 1 to words(canonical_composed)
                        cp = word(canonical_composed, w)
                        ccc = canonical_combining_class.cp
                        if inert then mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc";"
                        else mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc"; multiple_inert["ix"] =" inert.cp";"
                        ix = ix + 1
                    end

                    /* Branch to starter_then_non_starter: */
                    if inert then do
                        call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp = (char*)'to_utf8_string(subword(canonical_composed,1,last_starter)) ||,
                            '; single_cp_len =' utf8_string_bytes(subword(canonical_composed,1,last_starter)) ||,
                            ';' mult 'goto inert_starter_then_non_starter;} // TP8' code_point'->'canonical_composed
                        type_count._CAN_RECOM_INERT_STARTER_THEN_NON_STARTER = type_count._CAN_RECOM_INERT_STARTER_THEN_NON_STARTER + 1
                    end
                    else do
                        /* "goto full" target used in the non-inert scenario does not support single_cp must use multiple_cp[] for all codepoints */
                        call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {' mult 'goto full;} // TP9' code_point'->'canonical_composed
                        type_count._COM_DECOM_COMPLEX = type_count._COM_DECOM_COMPLEX + 1
                    end
                end

                else do
                    /*
                        TP10. Otherwise -> full
                    */
                    /* load multiple_cp[] etc. */
                    mult = "multiple_cp_count =" (words(canonical_composed))"; "
                    ix = 0

                    do w = 1 to words(canonical_composed)
                        cp = word(canonical_composed, w)
                        ccc = canonical_combining_class.cp
                        if inert then mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc";"
                        else mult = mult "multiple_cp["ix"] = (char*)"to_utf8(cp)"; multiple_cp_len["ix"] =" utf8_bytes(cp)"; multiple_ccc["ix"] =" ccc"; multiple_inert["ix"] =" inert.cp";"
                        ix = ix + 1
                    end

                    /* Branch */
                    call lineout out_file, '        "'escape(code_point,length(strip(code_point,"L","0")))'" {single_cp_len = 0;' mult 'goto full;} // TP10' code_point'->'canonical_composed
                    type_count._CAN_RECOM_COMPLEX = type_count._CAN_RECOM_COMPLEX + 1
                end
            end
        end
    end
end

/* Write the tail of the normaliser */
call write_nfkc_and_nfc_normaliser_tail

/* report how many records were processed */
say "Processed" codelist.0 "records"

say "No Canonical Decomposition Starter" type_count._NO_CAN_DECOM_STARTER
say "No Canonical Decomposition Non-Starter" type_count._NO_CAN_DECOM_NON_STARTER
say "Canonical Decomposition Starter" type_count._CAN_DECOM_STARTER
say "Canonical Decomposition Single" type_count._CAN_DECOM_SINGLE
say "Canonical Decomposition Starter Then Non-Starter" type_count._CAN_DECOM_STARTER_THEN_NON_STARTER
say "Canonical Decomposition Complex" type_count._CAN_DECOM_COMPLEX

say "No Compatibility Decomposition Starter" type_count._NO_COM_DECOM_STARTER
say "No Compatibility Decomposition Non-Starter" type_count._NO_COM_DECOM_NON_STARTER
say "Compatibility Decomposition Starter" type_count._COM_DECOM_STARTER
say "Compatibility Decomposition Single" type_count._COM_DECOM_SINGLE
say "Compatibility Decomposition Starter Then Non-Starter" type_count._COM_DECOM_STARTER_THEN_NON_STARTER
say "Compatibility Decomposition Complex" type_count._COM_DECOM_COMPLEX


/*  Procedure to write the tail of the NFKC and NFC normaliser
    These are the treatment classifications / goto targets (IS=Inert Starter S=Starter IX=Inert Single X=Single)
       TP1. IS -> inert_starter
       TP2. S -> starter
       TP3. IX -> inert_single
       TP5. IS (X|IX)+ IS -> inert_starter
       TP7. S (X|IX)+ (S|IS) -> multi_starter
       TP8. IS (IX)+ inert_starter_then_non_starter
       TP10. Otherwise -> full
*/
say "Canonical TP1  No Recomposition Inert Starter" type_count._NO_CAN_RECOM_INERT_STARTER
say "Canonical TP5     Recomposition Inert Starter" type_count._CAN_RECOM_INERT_STARTER
say "Canonical TP2  No Recomposition Non-Inert Starter" type_count._NO_CAN_RECOM_NON_INERT_STARTER
say "Canonical TP2     Recomposition Non-Inert Starter" type_count._CAN_RECOM_STARTER
say "Canonical TP7     Recomposition Multi Starter" type_count._CAN_RECOM_MULTI_STARTER
say "Canonical TP3  No Recomposition Inert Single" type_count._NO_CAN_RECOM_INERT_NON_STARTER
say "Canonical TP3     Recomposition Inert Single" type_count._CAN_RECOM_INERT_SINGLE
/* say "Canonical TP4  No Recomposition Non-Inert Single" type_count._NO_CAN_RECOM_NON_INERT_NON_STARTER */
/* say "Canonical TP4     Recomposition Non-Inert Single" type_count._CAN_RECOM_SINGLE */
say "Canonical TP8     Recomposition Inert Starter Then Non-Starter" type_count._CAN_RECOM_INERT_STARTER_THEN_NON_STARTER
/* say "Canonical TP9     Recomposition Non-Inert Starter Then Non-Starter" type_count._CAN_RECOM_STARTER_THEN_NON_STARTER */
say "Canonical TP10    Full" type_count._CAN_RECOM_COMPLEX + type_count._NO_CAN_RECOM_NON_INERT_NON_STARTER

exit 0

/* Procedure to write the tail of the NFKD and NFD normaliser */
write_nfkd_and_nfd_normaliser_tail: procedure expose out_file
    call lineout out_file, ""
    call lineout out_file, "        // Anything else"
    call lineout out_file, "        any {"
    call lineout out_file, '           printf("ERROR [%.*s]\n", (int)(str - start), start);'
    call lineout out_file, '           return 1;'
    call lineout out_file, "        }"
    call lineout out_file, ""
    call lineout out_file, "        // Invalid stuff"
    call lineout out_file, '        * { printf("ERROR invalid input [%.*s]\n", (int)(str - start), start); return 1; }'
    call lineout out_file, ""
    call lineout out_file, "        // End of Input"
    call lineout out_file, "        $   {"
    call lineout out_file, "                break;"
    call lineout out_file, "            }"
    call lineout out_file, "*/"
    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for single codepoint starter (ccc = 0) or multiple codepoints starting and ending with a starter */"
    call lineout out_file, "starter:"
    call lineout out_file, "        /* Write out any queued cps */"
    call lineout out_file, "        for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "            for (j = 0; j < queued_cp_len[i]; j++) {"
    call lineout out_file, "                out[(*out_len)++] = queued_cp[i][j];"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        queued_cps = 0;"
    call lineout out_file, "        /* Write out single codepoint */"
    call lineout out_file, "        for (i = 0; i < single_cp_len; i++) {"
    call lineout out_file, "            out[(*out_len)++] = single_cp[i];"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Continue loop */"
    call lineout out_file, "        continue;"
    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for single codepoint non-starter (ccc <> 0) */"
    call lineout out_file, "single:"
    call lineout out_file, "        /* Add to queue - using the ccc to place the new element in the right place (a bit like a bubble sort) */"
    call lineout out_file, "        for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "            if (single_ccc < queued_cp_ccc[i]) {"
    call lineout out_file, "                /* Move all elements up one */"
    call lineout out_file, "                for (j = queued_cps; j > i; j--) {"
    call lineout out_file, "                    queued_cp[j] = queued_cp[j-1];"
    call lineout out_file, "                    queued_cp_len[j] = queued_cp_len[j-1];"
    call lineout out_file, "                    queued_cp_ccc[j] = queued_cp_ccc[j-1];"
    call lineout out_file, "                }"
    call lineout out_file, "                /* Insert new element */"
    call lineout out_file, "                queued_cp[i] = single_cp;"
    call lineout out_file, "                queued_cp_len[i] = single_cp_len;"
    call lineout out_file, "                queued_cp_ccc[i] = single_ccc;"
    call lineout out_file, "                queued_cps++;"
    call lineout out_file, "                break;"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        /* If not inserted then add to end */"
    call lineout out_file, "        if (i == queued_cps) {" /* i is set in the for loop above */
    call lineout out_file, "            queued_cp[queued_cps] = single_cp;"
    call lineout out_file, "            queued_cp_len[queued_cps] = single_cp_len;"
    call lineout out_file, "            queued_cp_ccc[queued_cps] = single_ccc;"
    call lineout out_file, "            queued_cps++;"
    call lineout out_file, "        }"
    call lineout out_file, "        /* continue loop */"
    call lineout out_file, "        continue;"
    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for codepoints (cps) starting with a starter (ccc = 0) */"
    call lineout out_file, "starter_then_non_starter:"
    call lineout out_file, "        /* Write out any queued cps */"
    call lineout out_file, "        for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "            for (j = 0; j < queued_cp_len[i]; j++) {"
    call lineout out_file, "                out[(*out_len)++] = queued_cp[i][j];"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        queued_cps = 0;"
    call lineout out_file, "        /* Write out single codepoint */"
    call lineout out_file, "        for (i = 0; i < single_cp_len; i++) {"
    call lineout out_file, "            out[(*out_len)++] = single_cp[i];"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Copy multiple cps to queue */"
    call lineout out_file, "        for (i = 0; i < multiple_cp_count; i++) {"
    call lineout out_file, "            queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "            queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "            queued_cp_ccc[queued_cps] = multiple_ccc[i];"
    call lineout out_file, "            queued_cps++;"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Continue loop */"
    call lineout out_file, "        continue;"
    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for complex cases (cps before starter, cps between starters, cps following last starter */"
    call lineout out_file, "complex:"
    call lineout out_file, "        /* loop round multiple cps */"
    call lineout out_file, "        for (i = 0; i < multiple_cp_count; i++) {"
    call lineout out_file, "            if (multiple_ccc[i] == 0) {" /* Starter */
    call lineout out_file, "                /* Write out any queued cps */"
    call lineout out_file, "                for (j = 0; j < queued_cps; j++) {"
    call lineout out_file, "                    for (k = 0; k < queued_cp_len[j]; k++) {"
    call lineout out_file, "                        out[(*out_len)++] = queued_cp[j][k];"
    call lineout out_file, "                    }"
    call lineout out_file, "                }"
    call lineout out_file, "                queued_cps = 0;"
    call lineout out_file, "                /* Write out multiple_ccc[i] */"
    call lineout out_file, "                for (j = 0; j < multiple_cp_len[i]; j++) {"
    call lineout out_file, "                    out[(*out_len)++] = multiple_cp[i][j];"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "            else {" /* Non-starter */
    call lineout out_file, "                /* Add to queue - using the ccc to place the new element in the right place (a bit like a bubble sort) */"
    call lineout out_file, "                for (j = 0; j < queued_cps; j++) {"
    call lineout out_file, "                    if (multiple_ccc[i] < queued_cp_ccc[j]) {"
    call lineout out_file, "                        /* Move all elements up one */"
    call lineout out_file, "                        for (k = queued_cps; k > j; k--) {"
    call lineout out_file, "                            queued_cp[k] = queued_cp[k-1];"
    call lineout out_file, "                            queued_cp_len[k] = queued_cp_len[k-1];"
    call lineout out_file, "                            queued_cp_ccc[k] = queued_cp_ccc[k-1];"
    call lineout out_file, "                        }"
    call lineout out_file, "                        /* Insert new element */"
    call lineout out_file, "                        queued_cp[j] = multiple_cp[i];"
    call lineout out_file, "                        queued_cp_len[j] = multiple_cp_len[i];"
    call lineout out_file, "                        queued_cp_ccc[j] = multiple_ccc[i];"
    call lineout out_file, "                        queued_cps++;"
    call lineout out_file, "                        break;"
    call lineout out_file, "                    }"
    call lineout out_file, "                }"
    call lineout out_file, "                /* If not inserted then add to end */"
    call lineout out_file, "                if (j == queued_cps) {" /* j is set in the for loop above */
    call lineout out_file, "                    queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "                    queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "                    queued_cp_ccc[queued_cps] = multiple_ccc[i];"
    call lineout out_file, "                    queued_cps++;"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, ""
    call lineout out_file, "    }"
    call lineout out_file, ""
    call lineout out_file, "    /* Write out any queued cps */"
    call lineout out_file, "    for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "        for (j = 0; j < queued_cp_len[i]; j++) {"
    call lineout out_file, "            out[(*out_len)++] = queued_cp[i][j];"
    call lineout out_file, "        }"
    call lineout out_file, "    }"
    call lineout out_file, ""
    call lineout out_file, "    return 0;"
    call lineout out_file, "}"
    call lineout out_file, ""
    return

/*  Procedure to write the tail of the NFKC and NFC normaliser
    These are the treatment classifications / goto targets (IS=Inert Starter S=Starter IX=Inert Single X=Single)
       TP1. IS -> inert_starter
       TP2. S -> starter
       TP3. IX -> inert_single
       TP5. IS (X|IX)+ IS -> inert_starter
       TP7. S (X|IX)+ (S|IS) -> multi_starter
       TP8. IS (IX)+ inert_starter_then_non_starter
       TP10. Otherwise -> full
*/
write_nfkc_and_nfc_normaliser_tail: procedure expose out_file
    call lineout out_file, ""
    call lineout out_file, "        // Anything else"
    call lineout out_file, "        any {"
    call lineout out_file, '           printf("ERROR [%.*s]\n", (int)(str - start), start);'
    call lineout out_file, '           return 1;'
    call lineout out_file, "        }"
    call lineout out_file, ""
    call lineout out_file, "        // Invalid stuff"
    call lineout out_file, '        * { printf("ERROR invalid input [%.*s]\n", (int)(str - start), start); return 1; }'
    call lineout out_file, ""
    call lineout out_file, "        // End of Input"
    call lineout out_file, "        $   {"
    call lineout out_file, "                break;"
    call lineout out_file, "            }"
    call lineout out_file, "*/"
    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for single inert codepoint starter (ccc = 0) or multiple inert codepoints starting and ending with a starter */"
    call lineout out_file, "        /* In Unicode 15 this covers about 44200 codepoints */"
    call lineout out_file, "inert_starter:"
    call lineout out_file, 'printf("inert_starter\n");'
    call lineout out_file, "        /* Write out any queued cps */"
    call lineout out_file, "        for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "            for (j = 0; j < queued_cp_len[i]; j++) {"
    call lineout out_file, "                out[(*out_len)++] = queued_cp[i][j];"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        queued_cps = 0;"
    call lineout out_file, "        /* Write out starter codepoints */"
    call lineout out_file, "        for (i = 0; i < single_cp_len; i++) {"
    call lineout out_file, "            out[(*out_len)++] = single_cp[i];"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Continue loop */"
    call lineout out_file, "        continue;"

    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for single non-inert codepoint starter (ccc = 0) */"
    call lineout out_file, "        /* In Unicode 15 this covers 874 codepoints */"
    call lineout out_file, "starter:"
    call lineout out_file, 'printf("starter\n");'
    call lineout out_file, "        if (queued_cps == 0) {"
    call lineout out_file, "            /* In this case we can just queue the starter - which might be combined with the next */"
    call lineout out_file, "            queued_cp[0] = single_cp;"
    call lineout out_file, "            queued_cp_len[0] = single_cp_len;"
    call lineout out_file, "            queued_cp_ccc[0] = 0;"
    call lineout out_file, "            queued_cp_inert[0] = 0;"
    call lineout out_file, "            queued_cps = 1;"
    call lineout out_file, "            /* Continue loop */"
    call lineout out_file, "            continue;"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Otherwise we queue starter */"
    call lineout out_file, "        queued_cp[queued_cps] = single_cp;"
    call lineout out_file, "        queued_cp_len[queued_cps] = single_cp_len;"
    call lineout out_file, "        queued_cp_ccc[queued_cps] = 0;"
    call lineout out_file, "        queued_cp_inert[queued_cps] = 0;"
    call lineout out_file, "        queued_cps++;"
    call lineout out_file, "        /* Then do full recompose algorithm */"
    call lineout out_file, "        goto full_compose;"

    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for multiple codepoints starting and ending with a starter */"
    call lineout out_file, "        /* In Unicode 15 there are no examples of this (!) */"
    call lineout out_file, "multi_starter:"
    call lineout out_file, 'printf("multi_starter\n");'
    call lineout out_file, "        if (queued_cps == 0) {"
    call lineout out_file, "            /* In this case we can just write out the starter upto the last codepoint which may compose later */"
    call lineout out_file, "            /* Write out multiple cps */"
    call lineout out_file, "            for (i = 0; i < multiple_cp_count - 1; i++) {"
    call lineout out_file, "                for (j = 0; j < multiple_cp_len[i]; j++) {"
    call lineout out_file, "                    out[(*out_len)++] = multiple_cp[i][j];"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "            if (multiple_inert[i]) {"
    call lineout out_file, "                /* Last codepoint is inert - so write it out it wont compose */"
    call lineout out_file, "                for (j = 0; j < multiple_cp_len[i]; j++) {"
    call lineout out_file, "                    out[(*out_len)++] = multiple_cp[i][j];"
    call lineout out_file, "                }"
    call lineout out_file, "            } else {"
    call lineout out_file, "                /* Last codepoint, a starter, is not inert - so we queue it */"
    call lineout out_file, "                queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "                queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "                queued_cp_ccc[queued_cps] = 0;"
    call lineout out_file, "                queued_cp_inert[queued_cps] = 0;"
    call lineout out_file, "                queued_cps++;"
    call lineout out_file, "            }"
    call lineout out_file, "            /* Continue loop */"
    call lineout out_file, "            continue;"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Otherwise we queue the multiple starter */"
    call lineout out_file, "        for (i = 0; i < multiple_cp_count; i++) {"
    call lineout out_file, "            queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "            queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "            queued_cp_ccc[queued_cps] = multiple_ccc[i];"
    call lineout out_file, "            queued_cp_inert[queued_cps] = multiple_inert[i];;"
    call lineout out_file, "            queued_cps++;"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Then do full recompose algorithm */"
    call lineout out_file, "        goto full_compose;"

    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for inert non-starter (single) codepoints - IX */"
    call lineout out_file, "        /* In Unicode 15 there are 879 examples of this */"
    call lineout out_file, "inert_single:"
    call lineout out_file, 'printf("inert_single\n");'
    call lineout out_file, "        /* Add to queue - using the ccc to place the new element in the right place (a bit like a bubble sort) */"
    call lineout out_file, "        for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "            if (single_ccc < queued_cp_ccc[i]) {"
    call lineout out_file, "                /* Move all elements up one */"
    call lineout out_file, "                for (j = queued_cps; j > i; j--) {"
    call lineout out_file, "                    queued_cp[j] = queued_cp[j-1];"
    call lineout out_file, "                    queued_cp_len[j] = queued_cp_len[j-1];"
    call lineout out_file, "                    queued_cp_ccc[j] = queued_cp_ccc[j-1];"
    call lineout out_file, "                    queued_cp_inert[j] = queued_cp_inert[j-1];"
    call lineout out_file, "                }"
    call lineout out_file, "                /* Insert new element */"
    call lineout out_file, "                queued_cp[i] = single_cp;"
    call lineout out_file, "                queued_cp_len[i] = single_cp_len;"
    call lineout out_file, "                queued_cp_ccc[i] = single_ccc;"
    call lineout out_file, "                queued_cps++;"
    call lineout out_file, "                break;"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        /* If not inserted then add to end */"
    call lineout out_file, "        if (i == queued_cps) {" /* i is set in the for loop above */
    call lineout out_file, "            queued_cp[queued_cps] = single_cp;"
    call lineout out_file, "            queued_cp_len[queued_cps] = single_cp_len;"
    call lineout out_file, "            queued_cp_ccc[queued_cps] = single_ccc;"
    call lineout out_file, "            queued_cps++;"
    call lineout out_file, "        }"
    call lineout out_file, "        /* continue loop - no composition needed as it is inert */"
    call lineout out_file, "        continue;"

    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for codepoints (cps) starting with an inert starter (ccc = 0) */"
    call lineout out_file, "        /* In Unicode 15 there are 60 examples of this */"
    call lineout out_file, "inert_starter_then_non_starter:"
    call lineout out_file, 'printf("inert_starter_then_non_starter\n");'
    call lineout out_file, "        /* Write out any queued cps */"
    call lineout out_file, "        for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "            for (j = 0; j < queued_cp_len[i]; j++) {"
    call lineout out_file, "                out[(*out_len)++] = queued_cp[i][j];"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        queued_cps = 0;"
    call lineout out_file, "        /* Write out single codepoint - its inert so wont combine */"
    call lineout out_file, "        for (i = 0; i < single_cp_len; i++) {"
    call lineout out_file, "            out[(*out_len)++] = single_cp[i];"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Copy multiple cps to queue */"
    call lineout out_file, "        for (i = 0; i < multiple_cp_count; i++) {"
    call lineout out_file, "            queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "            queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "            queued_cp_ccc[queued_cps] = multiple_ccc[i];"
    call lineout out_file, "            queued_cp_inert[queued_cps] = multiple_inert[i];"
    call lineout out_file, "            queued_cps++;"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Continue loop - no composition needed as it is inert */"
    call lineout out_file, "        continue;"

    call lineout out_file, ""
    call lineout out_file, "        /* Branch target for complex cases (cps before starter, cps between starters, cps following last starter */"
    call lineout out_file, "        /* In Unicode 15 there are 43 examples of this */"
    call lineout out_file, "full: ;"
    call lineout out_file, 'printf("full\n");'
    call lineout out_file, "        int inert_mode = 1; /* If this stays true then composition will not be needed */"
    call lineout out_file, "        int last_starter = -2; /* Used to track the last starter in the queue. -1 means no starter, -2 means not determined yet */"
    call lineout out_file, "        /* Check if anything in the queue is not inert */"
    call lineout out_file, "        for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "            if (!queued_cp_inert[i]) {"
    call lineout out_file, "                inert_mode = 0;"
    call lineout out_file, "                break;"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        /* loop round multiple cps while in inert mode */"
    call lineout out_file, "        if (inert_mode) {"
    call lineout out_file, "            for (i = 0; i < multiple_cp_count; i++) {"
    call lineout out_file, "                if (!multiple_inert[i]) {"
    call lineout out_file, "                    inert_mode = 0;"
    call lineout out_file, "                    break;"
    call lineout out_file, "                }"
    call lineout out_file, "                if (multiple_ccc[i] == 0) { /* Starter */"
    call lineout out_file, "                    /* Write out any queued cps */"
    call lineout out_file, "                    for (j = 0; j < queued_cps; j++) {"
    call lineout out_file, "                        for (k = 0; k < queued_cp_len[j]; k++) {"
    call lineout out_file, "                            out[(*out_len)++] = queued_cp[j][k];"
    call lineout out_file, "                        }"
    call lineout out_file, "                    }"
    call lineout out_file, "                    queued_cps = 0;"
    call lineout out_file, "                    last_starter = -1; /* Because the queue is empty */"
    call lineout out_file, "                    /* Write out multiple_ccc[i] */"
    call lineout out_file, "                    for (j = 0; j < multiple_cp_len[i]; j++) {"
    call lineout out_file, "                        out[(*out_len)++] = multiple_cp[i][j];"
    call lineout out_file, "                    }"
    call lineout out_file, "                }"
    call lineout out_file, "                else { /* Non-starter */"
    call lineout out_file, "                    /* Add to queue - using the ccc to place the new element in the right place (a bit like a bubble sort) */"
    call lineout out_file, "                    /* First find the last starter in the queue */"
    call lineout out_file, "                    if (last_starter == -2) { /* not determined yet */"
    call lineout out_file, "                        last_starter = -1; /* No starter in queue */"
    call lineout out_file, "                        for (j = queued_cps - 1; j >= 0; j--) {"
    call lineout out_file, "                            if (queued_cp_ccc[j] == 0) { /* Starter */"
    call lineout out_file, "                                 last_starter = j;"
    call lineout out_file, "                                 break;"
    call lineout out_file, "                            }"
    call lineout out_file, "                        }"
    call lineout out_file, "                    }"
    call lineout out_file, "                    for (j = last_starter + 1; j < queued_cps; j++) {"
    call lineout out_file, "                        if (multiple_ccc[i] < queued_cp_ccc[j]) {"
    call lineout out_file, "                            /* Move all elements up one */"
    call lineout out_file, "                             for (k = queued_cps; k > j; k--) {"
    call lineout out_file, "                                queued_cp[k] = queued_cp[k-1];"
    call lineout out_file, "                                queued_cp_len[k] = queued_cp_len[k-1];"
    call lineout out_file, "                                queued_cp_ccc[k] = queued_cp_ccc[k-1];"
    call lineout out_file, "                                queued_cp_inert[k] = queued_cp_inert[k-1];"
    call lineout out_file, "                            }"
    call lineout out_file, "                            /* Insert new element */"
    call lineout out_file, "                            queued_cp[j] = multiple_cp[i];"
    call lineout out_file, "                            queued_cp_len[j] = multiple_cp_len[i];"
    call lineout out_file, "                            queued_cp_ccc[j] = multiple_ccc[i];"
    call lineout out_file, "                            queued_cp_inert[j] = multiple_inert[i];"
    call lineout out_file, "                            queued_cps++;"
    call lineout out_file, "                            break;"
    call lineout out_file, "                        }"
    call lineout out_file, "                    }"
    call lineout out_file, "                    /* If not inserted then add to end */"
    call lineout out_file, "                    if (j == queued_cps) {" /* j is set in the for loop above */
    call lineout out_file, "                        queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "                        queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "                        queued_cp_ccc[queued_cps] = multiple_ccc[i];"
    call lineout out_file, "                        queued_cp_inert[queued_cps] = multiple_inert[i];"
    call lineout out_file, "                        queued_cps++;"
    call lineout out_file, "                    }"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "        }"

    call lineout out_file, "        /* continue to loop round multiple cps afterwards - we are no longer in inert mode and so */"
    call lineout out_file, "        /* we have to keep codepoints in the queue to be handled by "full_compose" */"
    call lineout out_file, "        for ( ; i < multiple_cp_count; i++) {"
    call lineout out_file, "            if (multiple_ccc[i] == 0) { /* Starter */"
    call lineout out_file, "                queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "                queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "                queued_cp_ccc[queued_cps] = multiple_ccc[i];"
    call lineout out_file, "                queued_cp_inert[queued_cps] = multiple_inert[i];"
    call lineout out_file, "                last_starter = queued_cps;"
    call lineout out_file, "                queued_cps++;"
    call lineout out_file, "            }"
    call lineout out_file, "            else { /* Non-starter */"
    call lineout out_file, "                /* Add to queue - using the ccc to place the new element in the right place (a bit like a bubble sort) */"
    call lineout out_file, "                /* First find the last starter in the queue */"
    call lineout out_file, "                if (last_starter == -2) { /* not determined yet */"
    call lineout out_file, "                    last_starter = -1; /* No starter in queue */"
    call lineout out_file, "                    for (j = queued_cps - 1; j >= 0; j--) {"
    call lineout out_file, "                        if (queued_cp_ccc[j] == 0) { /* Starter */"
    call lineout out_file, "                             last_starter = j;"
    call lineout out_file, "                             break;"
    call lineout out_file, "                        }"
    call lineout out_file, "                    }"
    call lineout out_file, "                }"
    call lineout out_file, "                for (j = last_starter + 1; j < queued_cps; j++) {"
    call lineout out_file, "                    if (multiple_ccc[i] < queued_cp_ccc[j]) {"
    call lineout out_file, "                        /* Move all elements up one */"
    call lineout out_file, "                         for (k = queued_cps; k > j; k--) {"
    call lineout out_file, "                            queued_cp[k] = queued_cp[k-1];"
    call lineout out_file, "                            queued_cp_len[k] = queued_cp_len[k-1];"
    call lineout out_file, "                            queued_cp_ccc[k] = queued_cp_ccc[k-1];"
    call lineout out_file, "                            queued_cp_inert[k] = queued_cp_inert[k-1];"
    call lineout out_file, "                        }"
    call lineout out_file, "                        /* Insert new element */"
    call lineout out_file, "                        queued_cp[j] = multiple_cp[i];"
    call lineout out_file, "                        queued_cp_len[j] = multiple_cp_len[i];"
    call lineout out_file, "                        queued_cp_ccc[j] = multiple_ccc[i];"
    call lineout out_file, "                        queued_cp_inert[j] = multiple_inert[i];"
    call lineout out_file, "                        queued_cps++;"
    call lineout out_file, "                        break;"
    call lineout out_file, "                    }"
    call lineout out_file, "                }"
    call lineout out_file, "                /* If not inserted then add to end */"
    call lineout out_file, "                if (j == queued_cps) {" /* j is set in the for loop above */
    call lineout out_file, "                    queued_cp[queued_cps] = multiple_cp[i];"
    call lineout out_file, "                    queued_cp_len[queued_cps] = multiple_cp_len[i];"
    call lineout out_file, "                    queued_cp_ccc[queued_cps] = multiple_ccc[i];"
    call lineout out_file, "                    queued_cp_inert[queued_cps] = multiple_inert[i];"
    call lineout out_file, "                    queued_cps++;"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "        }"
    call lineout out_file, "        /* Continue loop - no composition needed if we are inert */"
    call lineout out_file, "        if (inert_mode) continue;"

    call lineout out_file, ""
    call lineout out_file, "        /* Do a full recompose algorithm */"
    call lineout out_file, "        full_compose: ;"
    call lineout out_file, 'printf("full_compose\n");'
    call lineout out_file, "            /* Algorithm Notes"
    call lineout out_file, "               Blocked:Let A and C be two characters in a coded character sequence <A,...C>.C is blocked from A if and only"
    call lineout out_file, "               if ccc(A) = 0 and there exists some character B between A and C in the coded character sequence,"
    call lineout out_file, "               i.e., <A, ... B, ... C>, and either ccc(B) = 0 or ccc(B) >= ccc(C)."
    call lineout out_file, ""
    call lineout out_file, "               Canonical Composition Algorithm: Starting from the second character in the coded character sequence (of a Canonical"
    call lineout out_file, "               Decomposition or Compatibility Decomposition) and proceeding sequentially to the final character, perform the following"
    call lineout out_file, "               steps:"
    call lineout out_file, ""
    call lineout out_file, "                  Seek back (left) in the coded character sequence from the character C to find the last Starter L preceding C in the"
    call lineout out_file, "                  character sequence."
    call lineout out_file, ""
    call lineout out_file, "                  If there is such an L, and C is not blocked from L, and there exists a Primary Composite P which is canonically"
    call lineout out_file, "                  equivalent to the sequence <L, C>, then replace L by P in the sequence and delete C from the sequence."
    call lineout out_file, ""
    call lineout out_file, "               When the algorithm completes, all Non-blocked Pairs canonically equivalent to a Primary Composite will have been"
    call lineout out_file, "               systematically replaced by those Primary Composites."
    call lineout out_file, "            */"
    call lineout out_file, "            int c = 0, l, b, blocked;"
    call lineout out_file, "            while (1) {"
    call lineout out_file, "                c++;"
    call lineout out_file, "                if (c >= queued_cps) break; /* Completed processing */"
    call lineout out_file, "                if (queued_cp_inert[c]) continue; /* Inert so wont combine */"
    call lineout out_file, "                /* Find the last starter L preceding C in the character sequence */"
    call lineout out_file, "                for (l = c - 1; l >= 0; l--) {"
    call lineout out_file, "                    if (queued_cp_ccc[l] == 0) break;"
    call lineout out_file, "                }"
    call lineout out_file, "                if (l == -1) continue; /* No starter found */"
    call lineout out_file, "                if (queued_cp_inert[l]) continue; /* Inert so wont combine */"
    call lineout out_file, "                /* Loop through the codepoints between L and C to see if C is blocked form L */"
    call lineout out_file, "                blocked = 0;"
    call lineout out_file, "                for (b = l + 1; b < c && !blocked; b++) {"
    call lineout out_file, "                    if (queued_cp_ccc[b] == 0) blocked = 1; /* B is a starter */"
    call lineout out_file, "                    if (queued_cp_ccc[b] > queued_cp_ccc[c]) blocked = 1; /* B is blocked from C */"
    call lineout out_file, "                }"
    call lineout out_file, "                if (blocked == 0) { /* C is not blocked from L */"
    call lineout out_file, "                    /* Find a Primary Composite P which is canonically equivalent to the sequence <L, C> */"
    call lineout out_file, "                    int inert = 0;"
    call lineout out_file, "                    int comp_len = 0;"
    call lineout out_file, "                    int ccc = 0;"
    call lineout out_file, "                    char* comp = canonical_composer(queued_cp[l], queued_cp[c], &comp_len, &inert, &ccc);"
    call lineout out_file, "                    if (comp) {"
    call lineout out_file, "                        /* Replace L by P in the sequence and delete C from the sequence */"
    call lineout out_file, "                        queued_cp[l] = comp;"
    call lineout out_file, "                        queued_cp_len[l] = comp_len;"
    call lineout out_file, "                        queued_cp_ccc[l] = ccc;"
    call lineout out_file, "                        queued_cp_inert[l] = inert;"
    call lineout out_file, "                        /* Move all elements up one */"
    call lineout out_file, "                        for (k = c; k < queued_cps - 1; k++) {"
    call lineout out_file, "                            queued_cp[k] = queued_cp[k+1];"
    call lineout out_file, "                            queued_cp_len[k] = queued_cp_len[k+1];"
    call lineout out_file, "                            queued_cp_ccc[k] = queued_cp_ccc[k+1];"
    call lineout out_file, "                            queued_cp_inert[k] = queued_cp_inert[k+1];"
    call lineout out_file, "                        }"
    call lineout out_file, "                        queued_cps--;"
    call lineout out_file, "                        /* Reset c to point to the new L */"
    call lineout out_file, "                        c = l;"
    call lineout out_file, "                    }"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "            /* Write out the queue keeping the characters after the last starter (if the last char is starter then it will be kept in the queue if it is not inert) */"
    call lineout out_file, "            /* First find the last starter in the queue */"
    call lineout out_file, "            last_starter = -1; /* No starter in queue */"
    call lineout out_file, "            for (j = queued_cps - 1; j >= 0; j--) {"
    call lineout out_file, "                if (queued_cp_ccc[j] == 0) { /* Starter */"
    call lineout out_file, "                     last_starter = j;"
    call lineout out_file, "                     break;"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "            /* We will write out queued characters upto last_starter (exclusive) */"
    call lineout out_file, "            /* If there is no last_start set it so we don't write out anything */"
    call lineout out_file, "            /* If last_start is inert increment last_start so we write that out too (is it wont compose) */"
    call lineout out_file, "            if (last_starter == -1) last_starter = 0;"
    call lineout out_file, "            else if (queued_cp_inert[last_starter]) last_starter++;"
    call lineout out_file, "            /* Write out any queued cps */"
    call lineout out_file, "            for (i = 0; i < last_starter; i++) {"
    call lineout out_file, "                for (j = 0; j < queued_cp_len[i]; j++) {"
    call lineout out_file, "                    out[(*out_len)++] = queued_cp[i][j];"
    call lineout out_file, "                }"
    call lineout out_file, "            }"
    call lineout out_file, "            queued_cps -= last_starter;"
    call lineout out_file, "            /* Continue loop */"

    call lineout out_file, ""
    call lineout out_file, "    }"
    call lineout out_file, ""
    call lineout out_file, "    /* Write out any queued cps */"
    call lineout out_file, "    for (i = 0; i < queued_cps; i++) {"
    call lineout out_file, "        for (j = 0; j < queued_cp_len[i]; j++) {"
    call lineout out_file, "            out[(*out_len)++] = queued_cp[i][j];"
    call lineout out_file, "        }"
    call lineout out_file, "    }"
    call lineout out_file, ""
    call lineout out_file, "    return 0;"
    call lineout out_file, "}"
    call lineout out_file, ""
    return

/* Convert a integer (char) to a C escaped hex string literal */
dec_to_c_hex: procedure
    parse arg char
    return '"\x'd2x(char,2)'"'

/* Procedure to convert a code_point to a utf-8 escaped string literal for C source */
to_utf8: Procedure
    parse arg code_point
    code_point = x2d(code_point)
    if code_point < 128 then
        return '"\x'd2x(code_point,2)'"'
    if code_point < 2048 then
        return '"\x'd2x(192 + (code_point % 64),2)'\x'd2x(128 + (code_point // 64),2)'"'
    if code_point < 65536 then
        return '"\x'd2x(224 + (code_point % 4096),2)'\x'd2x(128 + ((code_point % 64) // 64),2)'\x'd2x(128 + (code_point // 64),2)'"'
    if code_point < 2097152 then
        return '"\x'd2x(240 + (code_point % 262144),2)'\x'd2x(128 + ((code_point % 4096) // 64),2)'\x'd2x(128 + ((code_point % 64) // 64),2)'\x'd2x(128 + (code_point // 64),2)'"'
    return ""

/* Procedure to convert get the nth utf8 byte value of a codepoint */
utf8_byte: procedure
    parse arg code_point, byte
    code_point = x2d(code_point)
    if code_point < 128 then do
        return code_point
    end
    if code_point < 2048 then do
        if byte = 1 then return 192 + (code_point % 64)
        if byte = 2 then return 128 + (code_point // 64)
    end
    if code_point < 65536 then do
        if byte = 1 then return 224 + (code_point % 4096)
        if byte = 2 then return 128 + ((code_point % 64) // 64)
        if byte = 3 then return 128 + (code_point // 64)
    end
    if code_point < 2097152 then do
        if byte = 1 then return 240 + (code_point % 262144)
        if byte = 2 then return 128 + ((code_point % 4096) // 64)
        if byte = 3 then return 128 + ((code_point % 64) // 64)
        if byte = 4 then return 128 + (code_point // 64)
    end
    return 0

/* Procedure to covert a string made up of space delimited codepoints to a utf-8 escaped string literal for C source */
to_utf8_string: procedure
    parse arg utf8_string
    result = ""
    do i = 1 to words(utf8_string)
        result = result to_utf8(word(utf8_string, i))
    end
    return space(result)

/* Procedure to return the number of bytes in a utf-8 codepoint */
utf8_bytes: procedure
    parse arg code_point
    code_point = x2d(code_point)
    if code_point < 128 then
        return 1
    if code_point < 2048 then
        return 2
    if code_point < 65536 then
        return 3
    if code_point < 2097152 then
        return 4
    return 0

/* Procedure to return the total number of bytes in a utf-8 string made up of space delimited codepoints */
utf8_string_bytes: procedure
    parse arg utf8_string
    bytes = 0
    do i = 1 to words(utf8_string)
        bytes = bytes + utf8_bytes(word(utf8_string, i))
    end
    return bytes

/* "Recursively" process canonical_decomposition */
canonical_decompos: procedure expose canonical_decomposition. processed_canonical_decomposition. 
    arg codepoint

    /* If it has not already been decomposed */
    if processed_canonical_decomposition.codepoint = "" then do

        if canonical_decomposition.codepoint = "" then do
            /* No decomposition */
            processed_canonical_decomposition.codepoint = codepoint
        end

        else do
            /* Recursively decompose */
            result = ""

            decom.0 = words(canonical_decomposition.codepoint)
            do d = 1 to decom.0
                decom.d = word(canonical_decomposition.codepoint, d)
            end

            /* decompose each sub-codepoint */
            d_decom.0 = decom.0
            do w = 1 to d_decom.0
                d_decom.w = canonical_decompos(decom.w)
            end

            /* result is d_decom.1 d_decom.2 ... concatenated */
            do w = 1 to d_decom.0
                result = result d_decom.w
            end
            processed_canonical_decomposition.codepoint = space(result)
        end
    end

    return processed_canonical_decomposition.codepoint

/* Recursively process compatibility_decomposition */
compatibility_decompos: procedure expose compatibility_decomposition. processed_compatibility_decomposition.
    arg codepoint

    /* If it has not already been decomposed */
    if processed_compatibility_decomposition.codepoint = "" then do

        if compatibility_decomposition.codepoint = "" then do
            /* No decomposition */
            processed_compatibility_decomposition.codepoint = codepoint
        end

        else do
            /* Recursively decompose */
            result = ""

            decom.0 = words(compatibility_decomposition.codepoint)
            do d = 1 to decom.0
                decom.d = word(compatibility_decomposition.codepoint, d)
            end

            /* decompose each sub-codepoint */
            d_decom.0 = decom.0
            do w = 1 to d_decom.0
                d_decom.w = compatibility_decompos(decom.w)
            end

            /* result is d_decom.1 d_decom.2 ... concatenated */
            do w = 1 to d_decom.0
                result = result d_decom.w
            end
            processed_compatibility_decomposition.codepoint = space(result)
        end
    end

    return processed_compatibility_decomposition.codepoint

/* Function to decompose a Hangul syllable into 2 codepoints (for recursive processing) */
DecomposeHangul: procedure
  parse arg codePoint
  codePoint = x2d(codePoint)

  /* Hangul syllable range: U+AC00 to U+D7A3 */
  if codePoint < 44032 | codePoint > 55203 then do
    return '' /* Not a Hangul syllable */
  end

  /* Constants for Hangul decomposition */
  SBase = 44032
  LBase = 4352
  VBase = 4449
  TBase = 4519
  LCount = 19
  VCount = 21
  TCount = 28
  NCount = VCount * TCount /* 588 */

  /* Calculate the decomposed jamos */
  SIndex = codePoint - SBase
  L = LBase + SIndex % NCount;
  V = VBase + (SIndex // NCount) % TCount;
  T = TBase + SIndex // TCount

  /* If T is equal to TBase, it means there's no final consonant */
  if T = TBase then do
    return d2x(L) d2x(V)
  end
  else do
    /* Decompose only into LV (V combined with L) and T */
    LV = SBase + ((L - LBase) * NCount) + ((V - VBase) * TCount)
    return d2x(LV) d2x(T)
    /* Note - if you wanted to decompose into L, V and T then you would return d2x(L) d2x(V) d2x(T) */
  end


/* Escape a string to a C string literal */
escape: procedure
   parse arg code, len

   if len<=2 then len=2
   else if len<=4 then len=4
   else if len<=8 then len=8

   code = right(lower(code), len, "0")
   select
      when len=2 then code = "\x"code
      when len=4 then code = "\u"code
      otherwise code = "\U"code
   end

   return code


/*
   Blocked:Let A and C be two characters in a coded character sequence <A,...C>.C is blocked from A if and only
   if ccc(A) = 0 and there exists some character B between A and C in the coded character sequence,
   i.e., <A, ... B, ... C>, and either ccc(B) = 0 or ccc(B) >= ccc(C).

   Canonical Composition Algorithm: Starting from the second character in the coded character sequence (of a Canonical
   Decomposition or Compatibility Decomposition) and proceeding sequentially to the final character, perform the following
   steps:

     Seek back (left) in the coded character sequence from the character C to find the last Starter L preceding C in the
     character sequence.

     If there is such an L, and C is not blocked from L, and there exists a Primary Composite P which is canonically
     equivalent to the sequence <L, C>, then replace L by P in the sequence and delete C from the sequence.

   When the algorithm completes, all Non-blocked Pairs canonically equivalent to a Primary Composite will have been
   systematically replaced by those Primary Composites.
*/
/* Procedure to return canonical unicode composition */
canonical_compose: procedure expose canonical_compos_ix. canonical_combining_class.
    arg codepoints
    if codepoints = "" then return ""
    if words(codepoints) = 1 then return codepoints
    c = 2
    do forever
        /* Find the last starter L preceding C in the character sequence */
        do l = c - 1 to 1 by -1
            l_cp = word(codepoints, l)
            if canonical_combining_class.l_cp = 0 then leave
        end
        if l = 0 then do
            c = c + 1 /* No starter found */
            if c > words(codepoints) then leave
            iterate
        end
        c_cp = word(codepoints, c)
        /* Loop through the codepoints between L and C */
        blocked = 0
        do b = l + 1 to c - 1 while blocked = 0
            b_cp = word(codepoints, b)
            if canonical_combining_class.b_cp = 0 then blocked = 1 /* B is a starter */
            if canonical_combining_class.b_cp > canonical_combining_class.c_cp then blocked = 1 /* B is blocked from C */
        end
        if blocked = 0 then do
            /* C is not blocked from L */
            /* Find a Primary Composite P which is canonically equivalent to the sequence <L, C> */
            decomposed_code_point = l_cp c_cp
            comp = canonical_compos_ix.decomposed_code_point
            if comp \= "" then do
                /* Replace L by P in the sequence and delete C from the sequence */
                pre_l = subword(codepoints, 1, l - 1)
                pre_c = subword(codepoints, l + 1, c - l - 1)
                post_c = subword(codepoints, c + 1)
                codepoints = pre_l comp pre_c post_c
                c = c - 1
            end
        end
        c = c + 1
        if c > words(codepoints) then leave
    end
    return space(codepoints)
