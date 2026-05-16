/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

options levelb
namespace rxcpexits expose parseexit

import rxcp
import rxfnsb

parseexit: class
    _node_id = .int
    _replacement = .string
    _template_kindtab = .string
    _template_texttab = .string
    _template = .string
    _wanttrim = .int
    _into = .string

    *: factory
        arg nid = .int
        _node_id = nid
        _replacement = ""
        _template_kindtab = ""
        _template_texttab = ""
        _template = ""
        _wanttrim = 0
        _into = ""

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("parse")
        call desc.add_flag("certified")
        call desc.add_flag("reserved_keyword")
        call desc.add_import("rxfnsb", "descriptor", "")
        return desc

pre_process: method = .exitplan
  arg tokens = .token[]
  compile_plan = .exitplan
  compile_plan = .exitplan("READY")

  _replacement = ""
  _template_kindtab = ""
  _template_texttab = ""
  _template = ""
  start_of_template=.int
  parmtype=""
  source_found=0
  uplow=.string
  wantlog=0
  wanttrace=0
  _wanttrim=0
  _into=''
 /* -------------------------------------------------------------------------------------------------------------------
  * Scan leading PARSE modifiers and source selector:
  * UPPER, LOWER, LOG, TRACE, TRIM, INTO, VALUE, VAR, ARG.
  * -------------------------------------------------------------------------------------------------------------------
  */
  do start_of_template=2 to tokens.0
     ti   = tokens[start_of_template]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     utext=upper(text)
     if isParseKeywordToken(ti, "UPPER") | isParseKeywordToken(ti, "LOWER") then do
        uplow=utext
        call compile_plan.add_keyword(start_of_template, text, "parse_option", "parse")
     end
     else if isParseKeywordToken(ti, "LOG") then do
        wantlog=1
        call compile_plan.add_keyword(start_of_template, text, "parse_option", "parse")
     end
     else if isParseKeywordToken(ti, "TRACE") then do
        wanttrace=1
        call compile_plan.add_keyword(start_of_template, text, "parse_option", "parse")
     end
     else if isParseKeywordToken(ti, "TRIM") then do
        _wanttrim=1
        call compile_plan.add_keyword(start_of_template, text, "parse_option", "parse")
     end
     else if isParseKeywordToken(ti, "INTO") then do
        call compile_plan.add_keyword(start_of_template, text, "parse_option", "parse")
        if start_of_template + 1 > tokens.0 then do
           call compile_plan.set_error(start_of_template, "PARSE INTO requires a target variable", "MISSING_ARGUMENTS")
           return compile_plan
        end
        ti = tokens[start_of_template+1]
        if isParseIdentifierToken(ti)=0 then do
           call compile_plan.set_error(start_of_template + 1, "PARSE INTO target must be an identifier", "INVALID_ARGUMENTS")
           return compile_plan
        end
        start_of_template = start_of_template + 1
        call compile_plan.add_binding("var", ti.get_text(), "", "", 0, "parse_into", "")
        _into = ti.get_text()
        iterate
     end
     else if isParseKeywordToken(ti, "VALUE") | isParseKeywordToken(ti, "VAR") | isParseKeywordToken(ti, "ARG") then do
        parmtype = utext
        call compile_plan.add_keyword(start_of_template, text, "parse_source", "parse")
        if parmtype = "ARG" then do
           source_found = 1
           leave
        end
     end
     else if isParseIdentifierToken(ti) & (parmtype='VAR' | parmtype='VALUE') then do
        source_found = 1
        leave
     end
     else if type='string_literal' & (parmtype='VAR' | parmtype='VALUE') then do
        source_found = 1
        leave
     end
  end

/* -------------------------------------------------------------------------------------------------------------------
 * Validate that source-based PARSE forms have a source expression.
 *
 * Statements using PARSE VALUE, PARSE VAR, or PARSE ARG require a source
 * operand. If the leading scan detected one of these keywords but did not
 * identify a corresponding source expression, compilation fails.
 * -------------------------------------------------------------------------------------------------------------------
 */
if parmtype \= "" & source_found = 0 then do
   call compile_plan.set_error(1, "PARSE " || parmtype || " requires a source expression", "MISSING_ARGUMENTS")
   return compile_plan
end

/* -------------------------------------------------------------------------------------------------------------------
 * For plain PARSE statements without an explicit source selector
 * (that is, no VALUE, VAR, or ARG), determine the first template token.
 *
 * The PARSE statement begins at token 1, so the template normally starts at
 * token 2. Leading modifiers and options are skipped:
 *
 *   UPPER / LOWER
 *   LOG
 *   TRACE
 *   TRIM
 *   INTO target    (occupies two tokens)
 * -------------------------------------------------------------------------------------------------------------------
 */
if parmtype = "" then do
   start_of_template = 2
   if uplow \= "" then start_of_template = start_of_template + 1
   if wantlog > 0 then start_of_template = start_of_template + 1
   if wanttrace > 0 then start_of_template = start_of_template + 1
   if _wanttrim > 0 then start_of_template = start_of_template + 1
   if _into \= "" then start_of_template = start_of_template + 2
end

/* -------------------------------------------------------------------------------------------------------------------
 * Ensure at least one template token remains after removing leading options.
 * -------------------------------------------------------------------------------------------------------------------
 */
if start_of_template > tokens.0 then do
   call compile_plan.set_error(1, "PARSE requires arguments", "MISSING_ARGUMENTS")
   return compile_plan
end

/* -------------------------------------------------------------------------------------------------------------------
 * PARSE ARG requires at least one template token following ARG.
 * -------------------------------------------------------------------------------------------------------------------
 */
if parmtype = "ARG" & start_of_template + 1 > tokens.0 then do
   call compile_plan.set_error(1, "PARSE requires arguments", "MISSING_ARGUMENTS")
   return compile_plan
end

/* -------------------------------------------------------------------------------------------------------------------
 * TRACE implies verbose parse logging.
 *
 * LOG enables standard diagnostics.
 * TRACE elevates the log level to 9.
 * -------------------------------------------------------------------------------------------------------------------
 */
if wanttrace > 0 then wantlog = 9

/* -------------------------------------------------------------------------------------------------------------------
 * Initialize working structures used to compile the PARSE template:
 *
 *   out               Number of compiled plan entries.
 *   _template         Reconstructed template text.
 *   pkind[]           Numeric token kind codes.
 *   ptext[]           Associated token text.
 *   template_end      Last token considered part of the template before
 *                     stripping trailing options such as INTO and TRIM.
 * -------------------------------------------------------------------------------------------------------------------
 */
out = 0
_template = ''
pkind = .int[]
ptext = .string[]
template_end = tokens.0

/* -------------------------------------------------------------------------------------------------------------------
 * Scan trailing PARSE options: INTO target and TRIM.
 * These may appear after the template and must be removed from template_end.
 * -------------------------------------------------------------------------------------------------------------------
 */
  do while template_end >= start_of_template + 1
     ti   = tokens[template_end]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     utext=upper(text)

     if isParseIdentifierToken(ti) & _into='' & template_end > start_of_template + 1 then do
        tj = tokens[template_end - 1]
        if isParseKeywordToken(tj, "INTO") then do
           call compile_plan.add_keyword(template_end - 1, tj.get_text(), "parse_option", "parse")
           call compile_plan.add_binding("var", text, "", "", 0, "parse_into", "")
           _into=text
           template_end = template_end - 2
           iterate
        end
     end

     if isParseKeywordToken(ti, "TRIM") & _wanttrim=0 then do
        call compile_plan.add_keyword(template_end, text, "parse_option", "parse")
        _wanttrim=1
        template_end = template_end - 1
        iterate
     end

     leave
  end
##  scan trailing PARSE options

   valstart=start_of_template
   split = find_parse_value_split(tokens, start_of_template, template_end)

   if split = 0 then do
      call compile_plan.set_error(1, ,
           "PARSE VALUE requires source expression and template", ,
           "MISSING_ARGUMENTS")
      return compile_plan
   end
   save=start_of_template
   varRange=split-1
   valClause=''
   do i=valstart to varRange
      ti   = tokens[i]
      type = strip(ti.get_type())
      text = strip(ti.get_text())
      valClause=valClause||'{'i'}'
   end
   start_of_template=split

/* -------------------------------------------------------------------------------------------------------------------
 * Scan the PARSE template tokens and build the executable parse plan.
 * -------------------------------------------------------------------------------------------------------------------
 * Compile the PARSE template into parallel kind/text arrays.
 *
 * Supported template tokens:
 *   identifier      Target variable
 *   .               Placeholder target, consumes output but assigns nothing
 *   string_literal  Literal delimiter
 *   int_literal     Absolute column position
 *   + int_literal   Relative forward column movement
 *   - int_literal   Relative backward column movement
 *   WITH            Optional separator, skipped once
 *
 * The compiler also inserts implicit markers:
 *
 *   - If the template starts with a target, an absolute position 1 marker is
 *     inserted before it.
 *   - If two targets are adjacent, kind 6 is inserted between them to represent
 *     the implicit blank-delimited parse boundary.
 *
 * Result arrays:
 *   pkind[n]  Numeric parse-plan token kind.
 *   ptext[n]  Associated token text.
 *
 * On invalid template syntax, this block records a compiler diagnostic and
 * returns the current exit plan.
 * -------------------------------------------------------------------------------------------------------------------
 */
  i = start_of_template
  haswith=0
  prevkind = 0

  do while i <= template_end
     ti   = tokens[i]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     tlen=length(text)

     if haswith=0 & isParseKeywordToken(ti, "WITH") then do
        haswith=1
        i = i + 1
        iterate
     end

     _template = _template || " " || text

     if type = "bracket" then do
        i = i + 1
        iterate
     end

     if type = "decimal_literal"  then do      /* is a token which belongs to the previous identifier, it is a tail of a stem in the form .number */
        ptext[out] = ptext[out]||text
        i = i + 1
        iterate
     end
     if type = "identifier" & substr(text,1,1)='.' & tlen>1  then do   /* is a token which belongs to the previous identifier, it is a tail of a stem in the form .variable */
        ptext[out] = ptext[out]||text                                  /* a single . is not part of a stem it is template drop identifier */
        i = i + 1
        iterate
     end
     if type = "int_literal" then do
        out = out + 1
        pkind[out] = 3
        ptext[out] = text
        prevkind = 3
        i = i + 1
        iterate
     end

     if type = "operator" then do
        if text = "+" | text = "-" then do
           if i + 1 <= tokens.0 then do
              tj    = tokens[i+1]
              ntype = strip(tj.get_type())
              ntext = strip(tj.get_text())
              if ntype = "int_literal" then do
                 out = out + 1
                 if text = "+" then pkind[out] = 4
                 else pkind[out] = 5
                 ptext[out] = ntext
                 prevkind = pkind[out]
                 i = i + 2
                 iterate
              end
           end
        end
        call compile_plan.set_error(1, "Invalid PARSE operator usage in template: " || text, "INVALID_ARGUMENTS")
        return compile_plan
     end

     if type = "string_literal" then do
        out = out + 1
        pkind[out] = 2
        ptext[out] = substr(text, 2, length(text) - 2)
        prevkind = 2
        i = i + 1
        iterate
     end

     if type = "identifier" then do
        if out = 0 then do
           out = out + 1
           pkind[out] = 3
           ptext[out] = "1"
           prevkind = 3
        end
        else if prevkind = 1 then do
           out = out + 1
           pkind[out] = 6
           ptext[out] = "{implicit}"
           prevkind = 6
        end
        out = out + 1
        pkind[out] = 1
        ptext[out] = text
        call compile_plan.add_binding("var", text, "", "", 0, "parse_target", "")
        prevkind = 1
        i = i + 1
        iterate
     end

     if type = "other" & text = "." then do
        if out = 0 then do
           out = out + 1
           pkind[out] = 3
           ptext[out] = "1"
           prevkind = 3
        end
        else if prevkind = 1 then do
           out = out + 1
           pkind[out] = 6
           ptext[out] = "{implicit}"
           prevkind = 6
        end
        out = out + 1
        pkind[out] = 1
        ptext[out] = "."
        prevkind = 1
        i = i + 1
        iterate
     end

     call compile_plan.set_error(1, "Unsupported token type in PARSE template: " || type, "INVALID_ARGUMENTS")
     return compile_plan
  end

  plan = compile_parse_plan(pkind, ptext, out)
  if substr(plan,1,8)=">>>ERROR" then do
     call compile_plan.set_error(1, substr(plan,9))
     return compile_plan
  end

  _template_kindtab=""
  _template_texttab=""
  do i = 1 to out
     _template_kindtab = _template_kindtab' 'pkind[i]
     if strip(ptext[i])='' then ptext[i]='?'
     _template_texttab = _template_texttab' 'ptext[i]
  end

  ti = start_of_template
  _replacement=''
  if parmtype = "ARG" then parse_string = build_parse_arg_source_expr()
  else do
     tj = tokens[ti]
     parse_string = strip(tj.get_text())
     parse_string =valClause
   end
  if _into='' then _into='_parseResult'
  if uplow='UPPER' then _replacement = _replacement"; _source=upper("parse_string")"
  else if uplow='LOWER' then _replacement = _replacement"; _source=lower("parse_string")"
  else _replacement = _replacement"; _source="parse_string
  _replacement = _replacement || '; ' || _into || '=parseExec(_source,' || quote_rexx_string(plan) || ',' || quote_rexx_string(_template) || ',' || wantlog || ')'
return compile_plan

/* -------------------------------------------------------------------------------------------------------------------
 * Finalizes the PARSE compiler exit by validating accepted token classes and
 * emitting the replacement source code prepared during pre_process().
 *
 * This method:
 *   1. Rejects token streams that are too short to be a PARSE statement.
 *   2. Verifies that every token type is supported by this exit.
 *   3. Ensures pre_process() has already built replacement code.
 *   4. Appends assignments from the parseExec() result array to each PARSE
 *      target variable.
 *   5. Applies STRIP() to target assignments when PARSE TRIM was requested.
 *
 * Target assignment mapping:
 *
 *   _template_kindtab entries with kind 1 represent PARSE targets.
 *   Each target maps to the next parseExec() result slot:
 *
 *       target = _into[n]
 *
 *   A dot target "." is a placeholder and consumes a result slot but does not
 *   generate an assignment.
 *
 * Returns:
 *   exitresult("REPLACE") with one generated replacement line, or an error.
 * -------------------------------------------------------------------------------------------------------------------
 */
process: method = .exitresult
  arg tokens = .token[]

    result = .exitresult("EMPTY")
    if tokens.0 < 3 then do
       call result.set_status("REJECT")
       return result
    end

    allowed = "identifier exit_keyword keyword int_literal decimal_literal string_literal operator comma other bracket"
    do i = 1 to tokens.0
        ti = tokens[i]
        t_type = strip(ti.get_type())
        if i = 1 then iterate
        if pos(t_type, allowed) > 0 then iterate
        call result.set_error(i, "Unsupported token type in PARSE: <" || t_type || "> text=<" || ti.get_text() || ">")
        return result
    end

    if _replacement = "" then do
        call result.set_error(1, "PARSE state missing before process")
        return result
    end

    replacement = _replacement
    j = 0
    wrds = words(_template_kindtab)
    do i = 1 to wrds
        if word(_template_kindtab, i) \= "1" then iterate
        j = j + 1
        var = word(_template_texttab, i)
        if var='.' then iterate
        if isVar(var)=0 then do
            call result.set_error(1, "PARSE invalid variable name=" || var)
            return result
        end
        if _wanttrim=0 then replacement = replacement '; 'var|| '='_into'[' || j || ']'
        else replacement = replacement '; 'var|| '=strip('_into'[' || j || '])'
    end
    call result.set_status("REPLACE")
    call result.add_replacement_line(replacement)
  return result
/* -------------------------------------------------------------------------------------------------------------------
 * Compiles the internal PARSE template representation into a serialized plan
 * string consumed by parseExec().
 *
 * Each template element is encoded as:
 *
 *     kind,length:text;
 *
 * where:
 *   kind    = Numeric token classification.
 *   length  = Character length of the associated text.
 *   text    = Token payload.
 *
 * Supported token kinds:
 *   1 = Target variable
 *   2 = Literal delimiter string
 *   3 = Absolute column position
 *   4 = Relative position (+n)
 *   5 = Relative position (-n)
 *   6 = Implicit positional marker inserted between adjacent variables
 *
 * Example:
 *
 *   pkind[1] = 3   ptext[1] = "1"
 *   pkind[2] = 1   ptext[2] = "name"
 *   pkind[3] = 2   ptext[3] = ","
 *
 * Produces:
 *
 *   "3,1:1;1,4:name;2,1:,;"
 *
 * The resulting plan string is interpreted at runtime by parseExec() to
 * execute the PARSE template efficiently without reparsing the original
 * source text.
 *
 * Parameters:
 *   pkind  Array containing token kind codes.
 *   ptext  Array containing token text.
 *   out    Number of populated entries in both arrays.
 *
 * Returns:
 *   Serialized plan string.
 *
 * Error Handling:
 *   If an unsupported token kind is encountered, a string beginning with
 *   ">>>ERROR" is returned so the caller can convert it into a compiler
 *   diagnostic.
 * -------------------------------------------------------------------------------------------------------------------
 */
compile_parse_plan: procedure = .string
  arg pkind=.int[], ptext=.string[], out=.int

  planStr = ""
  do i = 1 to out
     k = pkind[i]
     t = ptext[i]
     if k \= 1 & k \= 2 & k \= 3 & k \= 4 & k \= 5 & k \= 6 then do
        return ">>>ERROR PARSE COMPILE STREAM ERROR: INVALID TOKEN=" || k || " AT " || i
     end
     planStr = planStr || k || "," || length(t) || ":" || t || ";"
  end
return planStr

/* -------------------------------------------------------------------------------------------------------------------
 * Builds a CREXX expression that concatenates all arguments passed to the
 * current procedure into a single blank-delimited string.
 *
 * This is used to implement:
 *
 *     PARSE ARG ...
 *
 * The generated code performs the following steps:
 *
 *   1. Determine the number of arguments using ARG().
 *   2. Initialize an empty accumulator string.
 *   3. Loop over ARG[i] for i = 1 to ARG().
 *   4. Insert a single blank between arguments.
 *   5. Return the concatenated result using LEAVE WITH.
 *
 * Example:
 *
 *   If the procedure is called as:
 *
 *       myproc("abc", "def", "ghi")
 *
 *   the generated expression evaluates to:
 *
 *       "abc def ghi"
 *
 * The resulting expression is injected directly into generated replacement
 * source code and therefore must itself be a valid CREXX expression.
 *
 * Returns:
 *   A CREXX expression string that evaluates to the concatenated argument text.
 * -------------------------------------------------------------------------------------------------------------------
 */
build_parse_arg_source_expr: procedure = .string
  return "do; __rxcpx_parse_arg_count=arg(); __rxcpx_parse_arg_source=''; do __rxcpx_parse_arg_ix = 1 to __rxcpx_parse_arg_count; if __rxcpx_parse_arg_ix > 1 then __rxcpx_parse_arg_source=__rxcpx_parse_arg_source || ' '; __rxcpx_parse_arg_source=__rxcpx_parse_arg_source || arg[__rxcpx_parse_arg_ix]; end; leave with __rxcpx_parse_arg_source; end"

/* -------------------------------------------------------------------------------------------------------------------
 * Returns a CREXX single-quoted string literal representing the supplied text.
 *
 * Any embedded single quote characters are escaped by doubling them, which is
 * the standard CREXX string literal escaping rule:
 *
 *   Input:  It's fine
 *   Output: 'It''s fine'
 *
 * This helper is used when generating replacement source code so that arbitrary
 * text (for example parse plans or template strings) can be inserted safely into
 * generated CREXX statements.
 *
 * Parameters:
 *   raw  The unquoted source string.
 *
 * Returns:
 *   A valid single-quoted CREXX string literal.
 * -------------------------------------------------------------------------------------------------------------------
 */
quote_rexx_string: procedure = .string
  arg raw = .string

  quoted = "'"
  do i = 1 to length(raw)
     ch = substr(raw, i, 1)
     if ch = "'" then quoted = quoted || "''"
     else quoted = quoted || ch
  end
return quoted || "'"

/* -------------------------------------------------------------------------------------------------------------------
 * Validates that a string contains only characters permitted in a CREXX variable
 * name for PARSE target assignments.
 *
 * Allowed characters:
 *   - Letters A-Z and a-z
 *   - Digits 0-9
 *   - Period (.)
 *   - Underscore (_)
 *
 * This routine is intentionally permissive and checks only the character set.
 * It does not enforce additional language rules such as:
 *   - First-character restrictions
 *   - Reserved keyword exclusion
 *   - Compound variable syntax semantics
 *
 * It is used during PARSE code generation to ensure that generated assignment
 * targets contain only syntactically safe characters.
 *
 * Parameters:
 *   varname  Variable name to validate.
 *
 * Returns:
 *   1  All characters are valid.
 *   0  At least one invalid character was found.
 * -------------------------------------------------------------------------------------------------------------------
 */
isVar: procedure=.int
  arg varname=.string
  vlen=length(varname)
  do i = 1 to vlen
     c = substr(varname, i, 1)
     if pos(c, 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.') = 0 then return 0
  end
return 1

/* -------------------------------------------------------------------------------------------------------------------
 * Returns 1 if the token represents a user identifier.
 *
 * This helper is used to detect:
 *   - PARSE target variables
 *   - PARSE VAR source variables
 *   - PARSE INTO destination variables
 *
 * Only tokens whose type is exactly "identifier" are accepted.
 * Reserved words classified as "keyword" or "exit_keyword" are not treated
 * as identifiers.
 *
 * Parameters:
 *   tok  Token object to test.
 *
 * Returns:
 *   1  Token type is "identifier".
 *   0  Token is not an identifier.
 * -------------------------------------------------------------------------------------------------------------------
 */
isParseIdentifierToken: procedure = .int
  arg tok = .token
  if strip(tok.get_type()) = "identifier" then return 1
return 0

/* -------------------------------------------------------------------------------------------------------------------
 * Returns 1 if the token text matches the specified keyword and the token type is one
 * of the classes that may legally represent a keyword in the compiler token stream.
 *
 * Accepted token types:
 *   - identifier
 *   - keyword
 *   - exit_keyword
 *
 * This allows reserved words such as VALUE, WITH, ARG, INTO, etc. to be recognised
 * consistently even if the tokenizer classifies them differently depending on
 * compilation phase or grammar context.
 *
 * Parameters:
 *   tok      Token object to test.
 *   keyword  Keyword text to compare against (case-insensitive).
 *
 * Returns:
 *   1  Token represents the specified keyword.
 *   0  Token does not match.
 * -------------------------------------------------------------------------------------------------------------------
 */
isParseKeywordToken: procedure = .int
  arg tok = .token, keyword = .string
  type = strip(tok.get_type())
  text = upper(strip(tok.get_text()))
  if text \= upper(keyword) then return 0
  if type = "identifier" | type = "exit_keyword" | type = "keyword" then return 1
return 0
/* -------------------------------------------------------------------------------------------------------------------
 * Locate the split between PARSE VALUE source expression and template.
 *
 * Rules:
 * - Explicit WITH always separates source expression from template.
 * - Without WITH, the first complete top-level expression is the source.
 * - Therefore: PARSE VALUE a b c  => source=a, template=b c
 * - To use a multi-token source expression, code WITH:
 *   PARSE VALUE a b WITH c        => source=a b, template=c
 * -------------------------------------------------------------------------------------------------------------------
 */
find_parse_value_split: procedure = .int
  arg tokens = .token[], first = .int, last = .int

  paren_depth = 0
  square_depth = 0
  seen_expr = 0

  do i = first to last
     ti = tokens[i]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     tlen=length(text)
     if substr(text,1,1)='.' & tlen>1 then iterate  /* it's a stem, if the token contains . and the following tail, if tlen=1 it's maybe already the template */

     if paren_depth = 0 & square_depth = 0 & isParseKeywordToken(ti, "WITH") then return i

     if type = "bracket" then do
        if text = "(" then paren_depth = paren_depth + 1
        else if text = ")" & paren_depth > 0 then do
           paren_depth = paren_depth - 1
           if paren_depth = 0 & square_depth = 0 & seen_expr & i < last then return i + 1
        end
        else if text = "[" then square_depth = square_depth + 1
        else if text = "]" & square_depth > 0 then do
           square_depth = square_depth - 1
           if paren_depth = 0 & square_depth = 0 & seen_expr & i < last then return i + 1
        end

        seen_expr = 1
        iterate
     end

     if paren_depth = 0 & square_depth = 0 & seen_expr then return i
     seen_expr = 1
  end
return 0