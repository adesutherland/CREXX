options levelb
namespace rxcpexits expose parseexit _status _error_token _error_message start_of_template pkind ptext parse_preprocess

import rxcp
import rxfnsb

parseexit: class
    _node_id       = .int    with register.1
    _replacement   = .string with register.2
    _error_token   = .int    with register.3
    _error_message = .string with register.4
    _status        = .string with register.5

    /* ------------------------------------------------------------------------
     * Factory
     * ------------------------------------------------------------------------
     * Initialise a new exit instance for a compiler node.
     * ----------------------------------------------------------------------
     */
    *: factory
        arg nid = .int
        _node_id = nid
        _replacement = ""
        _error_token = 0
        _error_message = ""
        _status = "EMPTY"
    /* ------------------------------------------------------------------------
     * Primary keyword handled by this exit.
     * ----------------------------------------------------------------------
     */
    get_primary_keyword: method = .string
        return "parse"
    /* ------------------------------------------------------------------------
     * Additional trigger keywords.
     * Empty for now: the exit is bound only to "add".
     * ----------------------------------------------------------------------
     */
    get_additional_keywords: method = .string
        return ""

/* ------------------------------------------------------------------
 * PARSE compile phase (pre_process)
 *
 * Purpose:
 *   Convert tokenised PARSE template into a compact internal form:
 *
 *     pkind[] : token classification
 *     ptext[] : token payload
 *
 * Token kinds:
 *   1 = variable / target (including "." for drop)
 *   2 = string literal (delimiter)
 *   3 = absolute position (n)
 *   4 = relative forward (+n)
 *   5 = relative backward (-n)
 *
 * Additionally:
 *   - Build list of variables to expose (toExpose)
 *   - Normalize template by injecting implicit ABS 1 if needed
 * ------------------------------------------------------------------
 */
pre_process: method = .string
  arg tokens = .token[]

  start_of_template=.int
  parse_preprocess=.string[]
  do start_of_template=2 to tokens.0
     ti   = tokens[start_of_template]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     utext=upper(text)
     call log 123" "start_of_template" "type" "text
     if type='identifier' & (utext='UPPER' | utext='LOWER') then do
        parse_preprocess.2=utext
     end
     else if type='identifier' & (utext='VALUE' | utext='VAR') then do
        parse_preprocess.3=utext
     end
     else if type='identifier' & parse_preprocess.3='VAR' then do
        parse_preprocess.4=text
        leave
     end
     else if type='string_literal' & parse_preprocess.3='VALUE' then do
        parse_preprocess.4=text
        leave
     end
  end



  out = 0
  toExpose = ""
  pkind = .int[]
  ptext = .string[]

  parse_preprocess.1= start_of_template    /* token number of string/variable */
  call log 'parse-details 'parse_preprocess.1' 'parse_preprocess.2" "parse_preprocess.3" "parse_preprocess.4

  i = start_of_template+1

  do while i <= tokens.0
     ti   = tokens[i]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     call log "Tokenise "i" "out+1 " " type " " text
 /* --------------------------------------------------------------
  * Absolute position (n)
  * --------------------------------------------------------------
  */
     if type = "int_literal" then do
        out = out + 1
        pkind[out] = 3
        ptext[out] = text
        i = i + 1
        iterate
     end
 /* --------------------------------------------------------------
  * Relative position (+n / -n)
  * Must be operator followed by int_literal
  * --------------------------------------------------------------
  */
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
                 i = i + 2
                 iterate
              end
           end
        end
        say "invalid operator usage in parse template:" text
        return ""
     end
 /* --------------------------------------------------------------
  * String literal (delimiter)
  * Remove surrounding quotes
  * --------------------------------------------------------------
  */
     if type = "string_literal" then do
        out = out + 1
        pkind[out] = 2
        ptext[out] = substr(text, 2, length(text) - 2)
        i = i + 1
        iterate
     end
 /* --------------------------------------------------------------
  * Identifier → receiving variable
  * Also record for exposure (by token index)
  * --------------------------------------------------------------
  */
     if type = "identifier" then do
        out = out + 1
        pkind[out] = 1
        ptext[out] = text
        toExpose = toExpose || i' '
        i = i + 1
        iterate
     end
 /* --------------------------------------------------------------
  * "." → drop target (treated like variable for now)
  * --------------------------------------------------------------
  */
     if type = "other" & text = "." then do
        out = out + 1
        pkind[out] = 1
        ptext[out] = "."
        i = i + 1
        iterate
     end
 /* --------------------------------------------------------------
  * Unsupported token type
  * --------------------------------------------------------------
  */
     say "unsupported parse token kind:" type
     return ""
  end
/* ------------------------------------------------------------------
 * Normalize template:
 * If first token is a receiving target, PARSE semantics imply
 * an implicit starting position at column 1.
 *
 * So:
 *    w1 ',' w2
 * becomes:
 *    1 w1 ',' w2
 * ------------------------------------------------------------------
 */
  if out > 0 then do
     if pkind[1] = 1 then do
        do j = out to 1 by -1
           pkind[j+1] = pkind[j]
           ptext[j+1] = ptext[j]
        end
        pkind[1] = 3
        ptext[1] = "1"
        out = out + 1
     end
  end
  call log "must be exposed " toExpose" templates "out
return toExpose

process: method = .string
    arg tokens = .token[]
    /* ------------------------------------------------------------------------
     * Per-call state reset
     * ------------------------------------------------------------------------
     * Important: the exit object is reused, so reset all return fields before
     * processing a new candidate sequence.
     * ----------------------------------------------------------------------
     */
    _replacement = ""
    _error_token = 0
    _error_message = ""
    _status = "EMPTY"
    if tokens.0 < 3 then do
       _status = "REJECT"
       return _status
    end

    /* ------------------------------------------------------------------------
     * Check 1: lexical sanity (broad acceptance)
     * ------------------------------------------------------------------------
     * For now we allow only a conservative set of token classes in the source
     * sequence. This protects the rewrite step from constructs we do not yet
     * model explicitly.
     *
     * Accepted token classes:
     *   - identifier
     *   - int_literal
     *   - string_literal
     *   - operator
     *   - bracket
     *   - comma
     *   - other
     *
     * Notes
     *   - token 1 is the keyword itself and is therefore skipped
     *   - the joined source text is retained in 'cmd' for diagnostics/future use
     * ----------------------------------------------------------------------
     */
    allowed = "identifier int_literal string_literal operator comma other"
    cmd = ""

    do i = 1 to tokens.0
        ti = tokens[i]
        t_type = strip(ti.get_type())
        cmd = cmd' 'ti.get_text()
        if i = 1 then iterate
        if pos(t_type, allowed) > 0 then iterate
        return setError("ERROR",i,"Unsupported token type in PARSE: <"t_type"> text=<"ti.get_text()">" )
    end
    cmd = substr(cmd, 2)      /* for internal use if needed */

    /* ------------------------------------------------------------------------
     * Check 2: dependency on identifier typing
     * ------------------------------------------------------------------------
     * If one of the operand tokens is still typed as .unknown, we defer the
     * rewrite until later compiler phases have resolved enough information.
     *
     * This avoids premature rewrites on unstable syntax/semantic state.
     * ******** identifiers will remain unknown as we create them
     *   MAYBE WE NEED one for PARSE VAR, BUT THIS WILL BE DONE LATER
     * ----------------------------------------------------------------------
     */
 /*
    do i = 2 to tokens.0
        ti = tokens[i]
        call log 112' 'i': <'ti.get_text()'> 'ti.get_type()' 'ti.get_value_type()
        if strip(ti.get_type()) = "identifier" & ti.get_value_type() = ".unknown" then do
           _status = "PENDING"
           return _status
        end
    end
  */
    /* ------------------------------------------------------------------------
     * Emit canonical replacement
       parse_preprocess.1 string/variable token to parse
       parse_preprocess.2 upper/lower
       parse_preprocess.3 VALUE/VAR
       parse_preprocess.4 string/variable to parse

     * ------------------------------------------------------------------------
     */
     _status = "REPLACE"
     _replacement=""
     tx=.int
     tx = parse_preprocess.1                /* string to parse is third on, AT LEAST FOR NOW */
     call log "987 "parse_preprocess.1" "tx" "c2x(tx)
     tj   = tokens[tx]
     type=strip(tj.get_type())
     text=strip(tj.get_text())
     call log "671 "type" "text
    ## text=substr(text,2,length(text)-2)  /* strip off quotes */
     kindtab=''
     texttab=''
     do i=1 to pkind[0]
        kindtab=kindtab' 'pkind[i]
     end
     do i=1 to ptext[0]
       texttab=texttab' 'ptext[i]
     end
     _replacement='abc[1]="abc_1"; abc[2]="abc_2" ; say "outer 1 <"abc[1]">"; say "outer 2 "abc[2]">"; do j=1 to 2; say "j="j" inner 1 <"abc[j]">" ; say "j="j" inner 2 <"abc[j]">"; end'
/*
     _replacement='_rs=parse_exec('text',"'kindtab'", "'texttab'")'
     _replacement=_replacement'; if 'pkind[2]'=1  then 'ptext[2]'=_rs[1]'
     _replacement=_replacement'; if 'pkind[4]'=1  then 'ptext[4]'=_rs[2]'
     _replacement=_replacement'; if 'pkind[6]'=1  then 'ptext[6]'=_rs[3]'
     _replacement=_replacement'; if 'pkind[8]'=1  then 'ptext[8]'=_rs[4]'

     _replacement=_replacement'; if 'pkind[10]'=1 then 'ptext[10]'=_rs[5]'
   /*
     _replacement=_replacement'; if 'pkind[12]'=1 then 'ptext[12]'=_rs[6]'
     _replacement=_replacement'; if 'pkind[14]'=1 then 'ptext[14]'=_rs[7]'
     _replacement=_replacement'; if 'pkind[16]'=1 then 'ptext[16]'=_rs[8]'
     _replacement=_replacement'; if 'pkind[18]'=1 then 'ptext[18]'=_rs[9]'
     _replacement=_replacement'; if 'pkind[20]'=1 then 'ptext[20]'=_rs[10]'
  */
     call log "injected code "_replacement
     assembler SETATTRS pkind,0    /* reset the token arrays */
     assembler SETATTRS ptext,0
  */
     return _status
    /* ------------------------------------------------------------------------
     * Accessors
     * ----------------------------------------------------------------------
     */
    get_replacement: method = .string
        return _replacement

    get_error_token: method = .int
        return _error_token

    get_error_message: method = .string
        return _error_message

    get_status: method = .string
        return _status

    get_node_id: method = .int
        return _node_id

/* ============================================================================
 * Helper: setError
 * ----------------------------------------------------------------------------
 * Stores error return parameters and returns the status code.
 *
 * Parameters
 *   status         compiler-exit status to return
 *   error_token    1-based token index associated with the error
 *   error_message  diagnostic text
 * ==========================================================================
 */
setError: procedure = .string
    arg status = "EMPTY", error_token = 0, error_message = "unknown"
    _status = status
    _error_token = error_token
    _error_message = error_message
return _status

/* ============================================================================
 * Helper: log
 * ----------------------------------------------------------------------------
 * Lightweight file logger for debugging the compiler exit.
 *
 * Notes
 *   - Keep logging side-effects cheap and safe.
 *   - Consider guarding logging behind an environment switch for normal builds.
 *   - Current implementation writes to a fixed Windows path.
 *   - Returning '' keeps the helper unobtrusive to callers.
 * ==========================================================================
 */
log: procedure = .int
    arg logtxt = .string
return 0
return lineout("c:\temp\pluginlog.txt", time()" "logtxt)