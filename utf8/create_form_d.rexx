/* REXX Program to process unicode database file (UnicopdeData.txt) to generate C/re2c normalisation routines */
/* This generated normal routines for the NFD and NFKD normalisation form */
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
out_file = "norm_d_unicode.re"
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
canonical_max_decomposition = 0
compatibility_max_decomposition = 0
canonical_max_recomposition = 0
compatibility_max_recomposition = 0
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

/* Generate file */
/* Write the head of the out_file */
call lineout out_file, "/* Unicode NFD and NFKD Normaliser - generated by create_form_d.rexx baseed on unicode database file (UnicopdeData.txt) */"
call lineout out_file, "/* create_form_d.rexx written by Adrian Sutherland (c) 2023 */"
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

/* Write out individual unicode decompositions where the unicode is decomposed */
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

/* Write out individual unicode decompositions where the unicode is decomposed */
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
