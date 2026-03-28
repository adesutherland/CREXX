options levelb
namespace rxcpexits expose parseexit

import rxcp
import rxfnsb

parseexit: class
    _node_id              = .int
    _replacement          = .string
    _error_token          = .int
    _error_message        = .string
    _status               = .string
    _template_kindtab     = .string
    _template_texttab     = .string
    _template             = .string

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
        _template_texttab = ""
    /* ------------------------------------------------------------------------
     * Primary keyword handled by this exit.
     * ----------------------------------------------------------------------
     */
    get_primary_keyword: method = .string
        return "parse"
    /* ------------------------------------------------------------------------
     * Additional trigger keywords.
     * Empty for now: the exit is bound only to "parse".
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
 *   6 = position not set
 *
 * Additionally:
 *   - Build list of variables to expose (toExpose)
 *   - Normalize template by injecting implicit ABS 1 if needed
 * ------------------------------------------------------------------
 */
pre_process: method = .string
  arg tokens = .token[]

  _replacement = ""
  _error_token = 0
  _error_message = ""
  _status = "EMPTY"
  _template_kindtab = ""
  _template_texttab = ""
  _template = ""
  start_of_template=.int
  parmtype=""
  uplow=.string
  wantlog=0
  wanttrace=0
  do start_of_template=2 to tokens.0
     ti   = tokens[start_of_template]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     utext=upper(text)
     if type='identifier' & (utext='UPPER' | utext='LOWER')    then uplow=utext
     else if type='identifier' & utext='LOG' then wantlog=1
     else if type='identifier' & utext='TRACE' then wanttrace=1
     else if type='identifier' & (utext='VALUE' | utext='VAR') then parmtype=utext
     else if type='identifier' & (parmtype='VAR' | parmtype='VALUE') then leave  /* next token is with or parse string */
     else if type='string_literal' & (parmtype='VAR' | parmtype='VALUE') then leave  /* next token is with or parse string */
   end
   if parmtype="" then do
      start_of_template=2
      if uplow\="" then start_of_template=start_of_template+1
      if wantlog>0 then start_of_template=start_of_template+1
      if wanttrace>0 then start_of_template=start_of_template+1
  end
  if wanttrace>0 then wantlog=9
  out = 0
  toExpose = ""
  _template=''
  pkind = .int[]
  ptext = .string[]

  i = start_of_template+1
  call log 'Template start at 'i" parm type '"parmtype"' LOG="wantlog
  haswith=0
  prevkind = 0
 call log "*********** New Parse *************"
  do while i <= tokens.0
     ti   = tokens[i]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     if haswith=0 & upper(text) = "WITH" then do
        haswith=1
        i = i + 1
        iterate
     end
     call log "Tokenise "i" "out+1 " " type " <"text">"
     _template=_template||" "||text
 /* --------------------------------------------------------------
  * Absolute position (n)
  * --------------------------------------------------------------
  */
     if type = "bracket" then do
        i=i+1
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
                 prevkind = pkind[out]
                 i = i + 2
                 iterate
              end
           end
        end
        call no_raise "syntax", "40.23", "invalid operator usage in parse template:" text
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
        prevkind = 2
        i = i + 1
        iterate
     end
 /* --------------------------------------------------------------
  * Identifier → receiving variable
  * Inject implicit sequence marker if needed
  * --------------------------------------------------------------
  */
     if type = "identifier" then do
        /* first target in template -> implicit absolute column 1 */
        if out = 0 then do
           out = out + 1
           pkind[out] = 3
           ptext[out] = "1"
           prevkind = 3
        end
        /* adjacent target after target -> implicit continuation */
        else if prevkind = 1 then do
           out = out + 1
           pkind[out] = 6
           ptext[out] = "{implicit}"
           prevkind = 6
        end
        out = out + 1
        pkind[out] = 1
        ptext[out] = text
        toExpose = toExpose || i' '
        prevkind = 1
        i = i + 1
        iterate
     end
 /* --------------------------------------------------------------
  * "." → drop target
  * Same implicit-sequence handling as for normal variables
  * --------------------------------------------------------------
  */
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
 /* --------------------------------------------------------------
  * Unsupported token type
  * --------------------------------------------------------------
  */
     call no_raise "syntax", "40.23", "unsupported parse token kind: "type
  end
  plan=compile_parse_plan(pkind, ptext, out)

  _template_kindtab=""
  _template_texttab=""
  do i = 1 to out
     _template_kindtab = _template_kindtab' 'pkind[i]     /* no longer exposed */
     if strip(ptext[i])='' then ptext[i]='?'
     _template_texttab = _template_texttab' 'ptext[i]
  end
  call log "kindTab '"_template_kindtab"'"
  call log "TextTab '"_template_texttab"'"
  call log "Pass plan '"plan"'"

  ti = start_of_template     /* token number of string/variable to parse  */
  tj = tokens[ti]
  _replacement=''
  parse_string = strip(tj.get_text())
  if uplow='UPPER'      then _replacement = _replacement"; _source=upper("parse_string")"
  else if uplow='LOWER' then _replacement = _replacement"; _source=lower("parse_string")"
  else _replacement = _replacement"; _source="parse_string
  _replacement = _replacement'; _rs=parseexec(_source,"'plan'","'_template'",'wantlog')'
  call log 'Pre-Process II  '_template_texttab
  call log 'Pre-Process IV  '_replacement
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
 ##   _replacement = ""     /* will be reset in pre-exit mode */
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
    allowed = "identifier int_literal string_literal operator comma other bracket"
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
    call log "initial command "cmd

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
     * Emit the replacement prepared during pre_process.
     *
     * The bridge now keeps one exit instance per node, so the preprocessed
     * template state remains attached to the object until process() runs.
     * ----------------------------------------------------------------------
     */
     _status = "REPLACE"
     call log 'Process 0 '_replacement
     if _replacement = "" then do
        return setError("ERROR", 1, "PARSE state missing before process")
     end

     call log 'Process IIa '_template_kindtab
     call log 'Process IIb '_template_texttab

     j = 0
     wrds=words(_template_kindtab)
     call log "KindTab "_template_kindtab
     do i = 1 to wrds
        if word(_template_kindtab,i)\="1" then iterate
         j = j + 1
         var=word(_template_texttab,i)
         call log 'Var is 'i' 'j' "'var'"'
         if var='.' then iterate
       #  else _replacement = _replacement || "_temp" || '=_rs[' || j || '];'
          else _replacement = _replacement '; 'var|| '=_rs[' || j || '];'
     end
     call log "Process III code "_replacement
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

/* ----------------------------------------------------------------------
 * compile_parse_plan
 *
 * Convert flat pkind[] / ptext[] token stream into a per-variable plan.
 *
 *
 *
 * Rules:
 *   - controls before a variable become that variable's start control
 *   - first control after a variable becomes that variable's end control
 *   - any further controls belong to the next variable's start side
 * ---------------------------------------------------------------------- */
compile_parse_plan: procedure = .string
  arg pkind=.int[], ptext=.string[], out=.int

  v = 0
  i = 1
  pendingKind = 0
  pendingText = ""

  planStr = ""

  do while i <= out

     /* gather start-side controls until variable */
     do while i <= out & pkind[i] \= 1
        if pkind[i] = 2 | pkind[i] = 3 | pkind[i] = 4 | pkind[i] = 5 | pkind[i] = 6 then do
           pendingKind = pkind[i]
           pendingText = ptext[i]
           i = i + 1
        end
        else call no_raise "syntax", "40.23","compile_parse_plan error: invalid token kind "pkind[i]" at "i
     end

     if i > out then leave

     if pkind[i] \= 1 then call no_raise "syntax", "40.23","compile_parse_plan error: variable expected at token "i
     v = v + 1

     startKind = pendingKind
     startText = pendingText
     varName   = ptext[i]
     endKind   = 0
     endText   = "0"

     call log "PLAN["v"] START=("startKind","startText") VAR="varName

     pendingKind = 0
     pendingText = ""

     i = i + 1

     /* first following control is end control */
     if i <= out then do
        if pkind[i] = 2 | pkind[i] = 3 | pkind[i] = 4 | pkind[i] = 5 | pkind[i] = 6 then do
           endKind = pkind[i]
           endText = ptext[i]
           call log "PLAN["v"] END=("endKind","endText")"
           i = i + 1
        end
     end

     /* additional controls become pending start for next variable */
     do while i <= out & pkind[i] \= 1
        if pkind[i] = 2 | pkind[i] = 3 | pkind[i] = 4 | pkind[i] = 5 | pkind[i] = 6 then do
           pendingKind = pkind[i]
           pendingText = ptext[i]
           call log "PENDING START=("pendingKind","pendingText") FROM TOKEN "i
           i = i + 1
        end
        else leave
     end

     planStr = planStr ,
             || startKind || "," ,
             || length(startText) || ":" || startText || "," ,
             || length(varName)   || ":" || varName   || "," ,
             || endKind || "," ,
             || length(endText)   || ":" || endText   || ";"
  end

  return planStr


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

No_raise: procedure
  arg p0=.string, p1=.string, p2=.string
  say "Error "p0 p1 p2
exit 8

/* ============================================================================
 * Helper: log
 * ----------------------------------------------------------------------------
 * Lightweight file logger for debugging the compiler exit.
 *
 * Notes
 *   - Keep logging side-effects cheap and safe.
 *   - "say" prints to rxc stdout (or maybe stderr?)
 *   - Consider guarding logging behind an environment switch for normal builds.
 *   - Commented out implementation writes to a fixed Windows path.
 * ==========================================================================
 */
log: procedure = .int
    arg logtxt = .string
    /* say "EXIT LOG >" logtxt */
   ## call lineout "c:\temp\pluginlog.txt", time() logtxt
    return 0
