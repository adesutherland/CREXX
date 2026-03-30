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
    _wanttrim             = .int

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
        _wanttrim=0
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
  _wanttrim=0

  do start_of_template=2 to tokens.0
     ti   = tokens[start_of_template]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     utext=upper(text)
     if type='identifier' & (utext='UPPER' | utext='LOWER')    then uplow=utext
     else if type='identifier' & utext='LOG' then wantlog=1
     else if type='identifier' & utext='TRACE' then wanttrace=1
     else if type='identifier' & utext='TRIM' then _wanttrim=1
     else if type='identifier' & (utext='VALUE' | utext='VAR') then parmtype=utext
     else if type='identifier' & (parmtype='VAR' | parmtype='VALUE') then leave  /* next token is with or parse string */
     else if type='string_literal' & (parmtype='VAR' | parmtype='VALUE') then leave  /* next token is with or parse string */
   end
   if parmtype="" then do
      start_of_template=2
      if uplow\="" then start_of_template=start_of_template+1
      if wantlog>0 then start_of_template=start_of_template+1
      if wanttrace>0 then start_of_template=start_of_template+1
      if _wanttrim>0 then start_of_template=start_of_template+1
  end
  if wanttrace>0 then wantlog=9
  out = 0
  toExpose = ""
  _template=''
  pkind = .int[]
  ptext = .string[]

  i = start_of_template+1
  call log 'TEMPLATE START AT='i" PARSE TYPE='"parmtype"' LOG="wantlog" TRIM="_wanttrim
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
        return setError("ERROR", 1, "PARSE invalid operator usage in template:" text)
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
     return setError("ERROR", 1, "PARSE unsupported parse token kind: "type)
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
            call log 'Var is 'i' 'j' "'var'" isVar='isvar(var)
         if var='.' then iterate
         if isvar(var)=0 then do
            _status = "ERROR"
            _error_token = 1
            _error_message = "PARSE invalid variable name="var
            return _status
        ##    return setError("ERROR", 1, "PARSE invalid variable name="var)
         end
         if _wanttrim=0 then _replacement = _replacement '; 'var|| '=_rs[' || j || ']'
         else _replacement = _replacement '; 'var|| '=strip(_rs[' || j || '])'
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
 * Purpose
 *   Transform the flat tokenized PARSE template representation
 *   (pkind[] / ptext[]) into a compiled per-variable execution plan.
 *
 *   The generated plan is serialized as a compact length-prefixed string
 *   and is intended to be consumed later by parse_exec_plan().
 *
 * Overview
 *   The PARSE template is initially available as a flat token stream:
 *
 *     pkind[i] = token kind
 *     ptext[i] = token text
 *
 *   This routine converts that flat stream into a logical sequence of
 *   variable-centered parse operations. Each output plan entry describes
 *   exactly one receiving variable and contains:
 *
 *     - start control kind
 *     - start control text
 *     - variable name
 *     - end control kind
 *     - end control text
 *
 *   The result is emitted as a serialized plan string so that runtime
 *   execution does not have to reconstruct token roles again.
 *
 * Input
 *   pkind=.int[]
 *     Flat array of token kinds produced by PARSE preprocessing.
 *
 *   ptext=.string[]
 *     Flat array of token texts corresponding to pkind[].
 *
 *   out=.int
 *     Highest token index / logical token count to process.
 *
 * Token kinds
 *   1 = receiving variable / target
 *   2 = literal delimiter
 *   3 = absolute cursor position
 *   4 = relative forward cursor shift
 *   5 = relative backward cursor shift
 *   6 = implicit next-word control
 *
 * Compile model
 *   The routine applies the following role-assignment rules:
 *
 *   1. Controls appearing before a variable become that variable's
 *      start control.
 *
 *   2. The first control appearing after a variable becomes that
 *      variable's end control.
 *
 *   3. Any additional controls after that are not merged or normalized;
 *      instead they are carried forward as the pending start control
 *      for the next variable.
 *
 *   This is the key semantic rule that allows templates such as:
 *
 *     6 q +6 -3 y
 *
 *   to compile correctly as:
 *
 *     q : start = 6     end = +6
 *     y : start = -3    end = none
 *
 *   rather than incorrectly collapsing the adjacent numeric controls.
 *
 * Pending control handling
 *   The routine keeps one pending start control in:
 *
 *     pendingKind
 *     pendingText
 *
 *   These represent the most recently seen control token that has not
 *   yet been assigned to a variable.
 *
 *   When a variable token is encountered:
 *
 *     startKind = pendingKind
 *     startText = pendingText
 *
 *   After assignment, pending control state is cleared.
 *
 *   If further control tokens appear after the variable's first end
 *   control, the last such control becomes the pending start control
 *   for the next variable.
 *
 * Output plan format
 *   One serialized entry is emitted per variable using the format:
 *
 *     startKind,startTextLen:startText,varNameLen:varName,
 *     endKind,endTextLen:endText;
 *
 *   Example:
 *
 *     3,1:6,1:q,4,1:6;5,1:3,1:y,0,1:0;
 *
 *   corresponding to:
 *
 *     q : start=(3,'6') end=(4,'6')
 *     y : start=(5,'3') end=(0,'0')
 *
 *   Length-prefix encoding is used so that blanks and arbitrary literal
 *   delimiters survive serialization unchanged. This is essential for
 *   cases such as blank delimiter parsing.
 *
 * Default values
 *   If a variable has no start control:
 *
 *     startKind = 0
 *     startText = ""
 *
 *   If a variable has no end control:
 *
 *     endKind = 0
 *     endText = "0"
 *
 *   endKind = 0 means "no end control" and is interpreted by runtime as
 *   remainder extraction.
 *
 * Diagnostics
 *   The routine emits log messages for:
 *
 *     - assigned start control
 *     - assigned end control
 *     - pending start control carried to the next variable
 *
 *   These diagnostics are useful when validating compile-time role
 *   assignment independently from runtime parse execution.
 *
 * Error handling
 *   A syntax error is raised if:
 *
 *     - a token kind outside the accepted control set is encountered
 *       while gathering controls
 *     - a variable token is expected but not found
 *
 * Runtime contract
 *   The returned plan string is the sole compile-time product consumed
 *   by parse_exec_plan(). Runtime must not reinterpret token adjacency
 *   or attempt to reassign semantic roles.
 *
 * Notes
 *   - Numeric control normalization must not be applied at this stage.
 *   - Adjacent numeric controls are semantically meaningful only after
 *     start/end role assignment.
 *   - This routine intentionally preserves authored literal text,
 *     including blanks.
 *
 * Returns
 *   planStr
 *     Serialized per-variable parse execution plan.
 * ----------------------------------------------------------------------
 */
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
          else return setError("ERROR", 1, "PARSE COMPILE PLAN ERROR: INVALID TOKEN="pkind[i]" AT "i)
       end

       if i > out then leave

       if pkind[i] \= 1 then return setError("ERROR", 1, "PARSE COMPILE PLAN ERROR: VARIABLE EXPECTED AT TOKEN="i)
       v = v + 1

       startKind = pendingKind
       startText = pendingText
       varName   = ptext[i]
       endKind   = 0
       endText   = "0"

       call log "PLAN["v"] START=("startKind","startText") VAR="varName

       /* start control has now been consumed */
       pendingKind = 0
       pendingText = ""

       i = i + 1

       /* first following control is end control */
       if i <= out then do
          if pkind[i] = 2 | pkind[i] = 3 | pkind[i] = 4 | pkind[i] = 5 | pkind[i] = 6 then do
             endKind = pkind[i]
             endText = ptext[i]
             call log "PLAN["v"] END=("endKind","endText")"
             /* -------------------------------------------------------
              * Logical duplication rule:
              * absolute positional boundaries are shared
              * between adjacent variables.
              *
              * Example:
              *   2 w1 2 w2 2 w3
              *
              * behaves internally like:
              *   2 w1 2 2 w2 2 2 w3
              *
              * so the same absolute position acts as:
              *   - end of current variable
              *   - start of next variable
              * ------------------------------------------------------- */
              shareIt = 0
              if pkind[i] = 3 then shareIt = 1
              else if (pkind[i] = 4 | pkind[i] = 5) & ptext[i] = "0" then shareIt = 1
              if shareIt then do
                 pendingKind = pkind[i]
                 pendingText = ptext[i]
                 call log "SHARED PENDING START=("pendingKind","pendingText") FROM END TOKEN "i
              end
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
 * Helper: isVAR test for valid Variable name, SYMBOL has some problems
 * ----------------------------------------------------------------------------
 */
isVar: procedure=.int
  arg varname=.string
  vlen=length(varname)
  do i = 1 to vlen
     c = substr(varname, i, 1)
     if pos(c, 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.') = 0 then return 0
  end
return 1

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
