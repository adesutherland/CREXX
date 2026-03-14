options levelb
namespace rxcpexits expose addexit _status _error_token _error_message

import rxcp
import rxfnsb

/* ============================================================================
 * Compiler Exit: ADD
 * ----------------------------------------------------------------------------
 * Purpose
 *   Rewrites the command-style source form
 *
 *       ADD <target> <value-expr>
 *
 *   into the canonical indexed assignment
 *
 *       <target>[<target>[0]+1] = <value-expr>
 *
 * Notes
 *   - The exit is intentionally conservative.
 *   - It waits until identifier typing is available before committing.
 *   - It accepts a broad but controlled token subset for the value expression.
 *   - The generated replacement is expression-preserving from token 3 onward.
 *
 * Example
 *   Source:
 *       ADD arr 42
 *
 *   Replacement:
 *       arr[arr[0]+1]=42
 *
 *   Source:
 *       ADD recs name||":"||value
 *
 *   Replacement:
 *       recs[recs[0]+1]=name||":"||value
 * ==========================================================================
 */

addexit: class
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
        return "add"
    /* ------------------------------------------------------------------------
     * Additional trigger keywords.
     * Empty for now: the exit is bound only to "add".
     * ----------------------------------------------------------------------
     */
    get_additional_keywords: method = .string
        return ""

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
    /* ------------------------------------------------------------------------
     * Minimum syntax shape
     * ------------------------------------------------------------------------
     * We require at least:
     *   token 1 = "ADD"
     *   token 2 = target
     *   token 3 = value / expression start
     *
     * If fewer than 3 tokens are present, this exit does not apply.
     * ----------------------------------------------------------------------
     */
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
    allowed = "identifier int_literal string_literal operator bracket comma other"
    cmd = ""

    do i = 1 to tokens.0
        ti = tokens[i]
        t_type = strip(ti.get_type())
        cmd = cmd' 'ti.get_text()
        if i = 1 then iterate
        if pos(t_type, allowed) > 0 then iterate
        return setError("ERROR",i,"Unsupported token type in ADD: <"t_type"> text=<"ti.get_text()">" )
    end
    cmd = substr(cmd, 2)      /* for internal use if needed */
    /* ------------------------------------------------------------------------
     * Check 2: dependency on identifier typing
     * ------------------------------------------------------------------------
     * If one of the operand tokens is still typed as .unknown, we defer the
     * rewrite until later compiler phases have resolved enough information.
     *
     * This avoids premature rewrites on unstable syntax/semantic state.
     * ----------------------------------------------------------------------
     */
    do i = 2 to tokens.0
        ti = tokens[i]
      /*  call log 112' 'i': <'ti.get_text()'> 'ti.get_type()' 'ti.get_value_type() */
        if strip(ti.get_type()) = "identifier" & ti.get_value_type() = ".unknown" then do
           _status = "PENDING"
           return _status
        end
    end
    /* ------------------------------------------------------------------------
     * Build the appended value expression
     * ------------------------------------------------------------------------
     * Everything from token 3 onward is treated as the right-hand-side
     * expression exactly as written, preserving operator sequences and spans.
     *
     * Examples:
     *   ADD arr 123
     *   ADD arr recs+10
     *   ADD arr name||":"||value
     * ----------------------------------------------------------------------
     */
    countExpr = ""
    do i = 3 to tokens[0]
        countExpr = countExpr || tokens[i].get_text()
    end
   /* ---------------------------------------------------------------------
    * Check 3: Semantic constraints.
    *   Parameters must be an array/stem type (contains '[') if defined in rule set.
    *   (Fix: check bracket in VALUE type, not lexical type.)
    * ----------------------------------------------------------------------
    */
     k=2             /* check first parameter must an array */
     if check_array(tokens[k].get_value_type())=0 then ,
        return setError("ERROR",k,k". parameter <"tokens[k].get_text()"> must be a stem/array")
    /* ------------------------------------------------------------------------
     * Emit canonical replacement
     * ------------------------------------------------------------------------
     * {2} refers to the second token text, i.e. the target collection name.
     *
     * Generated form:
     *     target[target[0]+1] = value-expr
     *
     * Assumption:
     *   target[0] stores the current logical element count.
     * ----------------------------------------------------------------------
     */
     _status = "REPLACE"
     _replacement = "{2}[{2}[0]+1]="countExpr
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
 * Helper: check_array
 * ----------------------------------------------------------------------------
 * check if provided parameter is an array
 */
check_array: procedure=.int
  arg array=.string
  t_val = strip(array)
  if pos("[", t_val)=0 then return 0
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
 *   - Consider guarding logging behind an environment switch for normal builds.
 *   - Current implementation writes to a fixed Windows path.
 *   - Returning '' keeps the helper unobtrusive to callers.
 * ==========================================================================
 */
log: procedure = .int
    arg logtxt = .string
return lineout("c:\temp\pluginlog.txt", time()" "logtxt)