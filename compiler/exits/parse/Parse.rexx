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
    _into                 = .string

    /* ------------------------------------------------------------------------
     * Factory
     * ------------------------------------------------------------------------
     * Initialise a new exit instance for one compiler node.
     *
     * Notes
     *   - One exit object is retained per parse node.
     *   - Runtime replacement text and diagnostics are reset here.
     * ---------------------------------------------------------------------- */
    *: factory
        arg nid = .int
        _node_id = nid
        _replacement = ""
        _error_token = 0
        _error_message = ""
        _status = "EMPTY"
        _template_texttab = ""
        _wanttrim=0
        _into=""

    /* ------------------------------------------------------------------------
     * Primary keyword handled by this exit.
     * ---------------------------------------------------------------------- */
    get_primary_keyword: method = .string
        return "parse"

    /* ------------------------------------------------------------------------
     * Additional trigger keywords.
     *
     * Empty for now: this exit is attached only to the PARSE keyword itself.
     * ---------------------------------------------------------------------- */
    get_additional_keywords: method = .string
        return ""

/* ------------------------------------------------------------------
 * PARSE compile phase (pre_process)
 *
 * Purpose
 *   Analyse the tokenised PARSE statement and prepare all information
 *   needed later by process():
 *
 *     1. Tokenise the PARSE template into flat arrays
 *          pkind[] : template item kinds
 *          ptext[] : template item payloads
 *
 *     2. Compile the flat template into a sequential runtime stream
 *        consumed by parseExec():
 *
 *          kind,len:text;
 *          kind,len:text;
 *          ...
 *
 *     3. Build the target exposure list returned to the compiler.
 *
 *     4. Prepare the replacement source code that will invoke parseExec()
 *        at runtime and assign returned values to target variables.
 *
 * Token kinds
 *   1 = variable / target (including "." for drop)
 *   2 = string literal
 *   3 = absolute position (n)
 *   4 = relative forward (+n)
 *   5 = relative backward (-n)
 *   6 = implicit next-word marker
 *
 * Notes
 *   - The compiler preserves authored template order.
 *   - Runtime semantics are resolved in parseExec(), not here.
 *   - Implicit ABS 1 is injected for a leading bare target.
 * ------------------------------------------------------------------ */
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
  _into=''

  /* --------------------------------------------------------------
   * Scan the PARSE prefix before the template proper.
   *
   * Recognised here:
   *   UPPER / LOWER  : source normalisation before runtime parsing
   *   LOG / TRACE    : runtime debug level
   *   TRIM           : strip assigned fields in process()
   *   VALUE / VAR    : source-selection mode
   *
   * Result
   *   start_of_template points to the token holding the parse source
   *   expression/variable; the actual template starts at +1.
   * -------------------------------------------------------------- */
  toExpose = ""
  do start_of_template=2 to tokens.0
     ti   = tokens[start_of_template]
     type = strip(ti.get_type())
     text = strip(ti.get_text())
     utext=upper(text)
     if type='identifier' & (utext='UPPER' | utext='LOWER')    then uplow=utext
     else if type='identifier' & utext='LOG' then wantlog=1
     else if type='identifier' & utext='TRACE' then wanttrace=1
     else if type='identifier' & utext='TRIM' then _wanttrim=1
     else if type='identifier' & utext='INTO' then do
        ti = tokens[start_of_template+1]
        if strip(ti.get_type())\='identifier' then iterate
        start_of_template=start_of_template+1
        toExpose=start_of_template||' '
        _into=ti.get_text()
        iterate
     end
     else if type='identifier' & (utext='VALUE' | utext='VAR') then parmtype=utext
     else if type='identifier' & (parmtype='VAR' | parmtype='VALUE') then leave  /* next token is with or parse string */
     else if type='string_literal' & (parmtype='VAR' | parmtype='VALUE') then leave  /* next token is with or parse string */
   end

   /* No explicit VALUE/VAR source mode:
    * source token position is derived from prefix option count.
    */
   if parmtype="" then do
      start_of_template=2
      if uplow\="" then start_of_template=start_of_template+1
      if wantlog>0 then start_of_template=start_of_template+1
      if wanttrace>0 then start_of_template=start_of_template+1
      if _wanttrim>0 then start_of_template=start_of_template+1
      if _into\="" then start_of_template=start_of_template+2
  end

  if wanttrace>0 then wantlog=9
  out = 0
  _template=''
  pkind = .int[]
  ptext = .string[]

  i = start_of_template+1
  call log 'TEMPLATE START AT='i" PARSE TYPE='"parmtype"' LOG="wantlog" TRIM="_wanttrim" INTO="_into
  haswith=0
  prevkind = 0
  call log "*********** New Parse *************"

  /* --------------------------------------------------------------
   * Tokenise the authored PARSE template into flat arrays.
   *
   * This preserves the source order exactly; parseExec() later
   * interprets the resulting sequential runtime stream.
   * -------------------------------------------------------------- */
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
  * Brackets
  *
  * Bracket tokens are currently ignored at template tokenisation level.
  * They occur around parsed source expressions but are not emitted as
  * runtime PARSE stream items.
  * -------------------------------------------------------------- */
     if type = "bracket" then do
        i=i+1
        iterate
     end

 /* --------------------------------------------------------------
  * Absolute position (n)
  * -------------------------------------------------------------- */
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
  *
  * Represented as operator followed by int_literal.
  * -------------------------------------------------------------- */
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
  * String literal
  *
  * Stored unquoted. Empty literals are preserved as empty text and
  * are handled explicitly by runtime parseExec().
  * -------------------------------------------------------------- */
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
  *
  * Rules
  *   - First bare target in a template implies ABS 1.
  *   - Adjacent targets imply implicit next-word parsing.
  *   - Target token positions are collected in toExpose for compiler
  *     variable exposure.
  * -------------------------------------------------------------- */
     if type = "identifier" then do
        /* first target in template -> implicit absolute column 1 */
        if out = 0 then do
           out = out + 1
           pkind[out] = 3
           ptext[out] = "1"
           prevkind = 3
        end
        /* adjacent targets imply implicit word parsing */
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
  *
  * Drop targets participate in PARSE sequencing like normal targets,
  * but are ignored later when assignment code is generated.
  * -------------------------------------------------------------- */
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
  * -------------------------------------------------------------- */
     return setError("ERROR", 1, "PARSE unsupported parse token kind: "type)
  end

  /* --------------------------------------------------------------
   * Compile flat token arrays into sequential runtime stream:
   *
   *   kind,len:text;
   *   kind,len:text;
   *   ...
   *
   * Runtime interpretation is deferred to parseExec().
   * -------------------------------------------------------------- */
  plan=compile_parse_plan(pkind, ptext, out)
   ##    call setError("ERROR",1,substr(plan,9))
  if substr(plan,1,8)=">>>ERROR" then do
     _status = "ERROR"
     _error_token = 1
     _error_message = substr(plan,9)
     return _status
  end

  /* --------------------------------------------------------------
   * Preserve the original flat token tables for process()-time
   * validation and target assignment generation.
   * -------------------------------------------------------------- */
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

  /* --------------------------------------------------------------
   * Build runtime replacement:
   *   - prepare source expression/variable
   *   - apply optional UPPER/LOWER transformation
   *   - call parseExec()
   * -------------------------------------------------------------- */
  ti = start_of_template     /* token number of string/variable to parse  */
  tj = tokens[ti]
  _replacement=''
  parse_string = strip(tj.get_text())
  if _into='' then _into='_parseResult'
  if uplow='UPPER'      then _replacement = _replacement"; _source=upper("parse_string")"
  else if uplow='LOWER' then _replacement = _replacement"; _source=lower("parse_string")"
  else _replacement = _replacement"; _source="parse_string
  _replacement = _replacement'; '_into'=parseExec(_source,"'plan'","'_template'",'wantlog')'
  call log 'Pre-Process II  '_template_texttab
  call log 'Pre-Process IV  '_replacement
  call log "must be exposed='"toExpose"' template='"out"'"
return toExpose

process: method = .string
    arg tokens = .token[]

    /* ------------------------------------------------------------------------
     * Per-call state reset
     *
     * The exit object is reused, so return diagnostics must be cleared for
     * every process() call.
     * ---------------------------------------------------------------------- */
 ##   _replacement = ""     /* will be reset in pre-exit mode */
    _error_token = 0
    _error_message = ""
    _status = "EMPTY"
    if tokens.0 < 3 then do
       _status = "REJECT"
       return _status
    end

    /* ------------------------------------------------------------------------
     * Check 1: lexical sanity
     *
     * Accept only a conservative token set until rewrite semantics are known
     * to be stable for all PARSE forms.
     *
     * Notes
     *   - token 1 is the PARSE keyword itself and is skipped
     *   - the joined command text is currently used only for logging
     * ---------------------------------------------------------------------- */
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
    cmd = substr(cmd, 2)
    call log "initial command "cmd

    /* ------------------------------------------------------------------------
     * Check 2: identifier typing deferral
     *
     * Historical / optional logic kept commented here:
     * identifier value-type checks may be reintroduced later if compiler
     * phase ordering requires rewrite deferral.
     * ---------------------------------------------------------------------- */
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
     * Emit the replacement prepared during pre_process().
     *
     * The bridge keeps one exit instance per node, so the preprocessed state
     * remains attached to this object until process() executes.
     * ---------------------------------------------------------------------- */
     _status = "REPLACE"
     call log 'Process 0 '_replacement
     if _replacement = "" then do
        return setError("ERROR", 1, "PARSE state missing before process")
     end

     call log 'Process IIa '_template_kindtab
     call log 'Process IIb '_template_texttab

     /* -----------------------------------------------------------
      * Emit one assignment per PARSE target variable.
      *
      * _rs[] is returned by parseExec() in target order.
      * Drop targets "." are skipped here.
      * Optional TRIM is applied at assignment time.
      * ----------------------------------------------------------- */
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
         if _wanttrim=0 then _replacement = _replacement '; 'var|| '='_into'[' || j || ']'
         else _replacement = _replacement '; 'var|| '=strip('_into'[' || j || '])'
     end
     call log "Process III code "_replacement
     return _status

    /* ------------------------------------------------------------------------
     * Accessors
     * ---------------------------------------------------------------------- */
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
 *   Serialize the tokenized PARSE template into the sequential runtime
 *   stream consumed by parseExec().
 *
 * Input
 *   pkind[] : token kinds produced by pre_process()
 *   ptext[] : token payloads produced by pre_process()
 *   out     : logical template item count
 *
 * Stream format
 *   One entry per template item in original authored order:
 *
 *     kind,len:text;
 *
 * Example
 *   3,1:1;1,1:a;2,1:,;1,1:b;
 *
 * Token kinds
 *   1 = variable / target
 *   2 = literal
 *   3 = absolute cursor position
 *   4 = relative forward cursor shift
 *   5 = relative backward cursor shift
 *   6 = implicit next-word control
 *
 * Notes
 *   - This routine no longer builds variable-centred start/end records.
 *   - Runtime sequencing and cursor semantics are handled in parseExec().
 *   - Literal payload text is preserved exactly, including blanks and
 *     empty-string literals.
 * ---------------------------------------------------------------------- */
 compile_parse_plan: procedure = .string
   arg pkind=.int[], ptext=.string[], out=.int

   planStr = ""
   do i = 1 to out
      k = pkind[i]
      t = ptext[i]

      /* compiler emits only runtime-supported stream items */
      if k \= 1 & k \= 2 & k \= 3 & k \= 4 & k \= 5 & k \= 6 then do
         return ">>>ERROR PARSE COMPILE STREAM ERROR: INVALID TOKEN="k" AT "i
      end

      /* preserve exact authored order for sequential runtime execution */
      planStr = planStr || k || "," || length(t) || ":" || t || ";"
   end
   call log '****** Plan V2="'Planstr'"'
   return planStr

/* ============================================================================
 * Helper: isVAR test for valid variable name
 *
 * Lightweight compiler-exit validation for generated PARSE assignment
 * targets. Kept local because SYMBOL()/related checks are not suitable
 * here for all compiler-exit situations.
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
 * Store compiler-exit diagnostic fields and return the selected status.
 *
 * Parameters
 *   status         compiler-exit status to return
 *   error_token    1-based token index associated with the error
 *   error_message  diagnostic text
 * ========================================================================== */
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
 *   - Current implementation writes to a fixed Windows path.
 *   - Suitable for local debugging, but platform-specific.
 *   - Keep logging side effects cheap; avoid depending on it for logic.
 * ========================================================================== */
log: procedure = .int
    arg logtxt = .string
    /* say "EXIT LOG >" logtxt */
    ## call lineout "c:\temp\pluginlog.txt", time() logtxt
return 0