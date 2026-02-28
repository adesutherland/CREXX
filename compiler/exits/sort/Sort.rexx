options levelb
namespace rxcpexits expose sortexit
import rxcp
import rxfnsb

/* =============================================================================
 * sortexit — compiler-exit implementing the "sort" keyword.
 *
 * Intended syntax (tokens):
 *   1: "sort" keyword (handled by get_primary_keyword)
 *   2: target stem/array identifier (must be an array/stem type)
 *   3: key / mode (identifier or int literal)
 *   4: order / options (identifier or string literal)
 *   5: optional numeric (identifier or int literal)
 *
 * Lifecycle:
 *   The compiler may call process() multiple times while types are still
 *   being inferred. We return:
 *     REJECT   -> not our syntax / insufficient tokens
 *     ERROR    -> syntax/type error; provide _error_token/_error_message
 *     PENDING  -> wait until identifier types are inferred
 *     REPLACE  -> provide replacement text via get_replacement()
 *
 * ==============================================================================
*/

sortexit: class
    _node_id        = .int    with register.1
    _replacement    = .string with register.2
    _error_token    = .int    with register.3
    _error_message  = .string with register.4
    _status         = .string with register.5

    *: factory
        arg nid = .int
        _node_id = nid
        _replacement = ""
        _error_token = 0
        _error_message = ""
        _status = "EMPTY"

    get_primary_keyword: method = .string
        return "sort"

    get_additional_keywords: method = .string
        return ""

    process: method = .string
        arg tokens = .token[]
      /* ----------------------------------------------------------------------
       * Rule set: allowed token *types* per argument position.
       * Note: token.get_type() is lexical ("identifier", "int_literal", etc.)
       *
       * Only simple atomic tokens are supported (identifier/int/string).
       *   Complex expressions produced by the tokenizer are intentionally
       *   not handled by this exit.
       * ----------------------------------------------------------------------
       */
      rule[2] = "identifier[]"                 /* must be an array identifier */
      rule[3] = "identifier/int_literal"       /* identifier or int literal*/
      rule[4] = "identifier/string_literal"    /* identifier or string literal*/
      rule[5] = "identifier/int_literal"       /* optional, see min-arg check below */

      /* ----------------------------------------------------------------------
       * Check 0: Minimal argument count.
       *   Your later code uses tokens[2..5], so require at least 5 tokens if
       *   parameter 5 is mandatory. If param 5 is optional, handle that below.
       * ----------------------------------------------------------------------
       */
        if tokens.0 < 4 then do
            _status = "REJECT"
            return _status
        end
        /* If param5 is mandatory, use: if tokens.0 < 5 then ERROR/REJECT */

      /* -----------------------------------------------------------------------
       * Check 1: Lexical token type validation (identifier/int/string literal).
       *   For each parameter position that exists, verify token.get_type()
       *   is in the allowed set defined in rule[].
       * -----------------------------------------------------------------------
       */
        do i = 2 to tokens.0
            if rule[i] = "" then iterate  /* ignore extra args not covered by rule[] */

            ti = tokens[i]
            t_type = strip(ti.get_type())
            if \_type_allowed(t_type, rule[i]) then do
                t_text = ti.get_text()
                _status = "ERROR"
                _error_token = i
                _error_message = i". parameter <"t_text"> must be <"rule[i]">, is <"t_type">"
                return _status
            end
        end
      /* ----------------------------------------------------------------------
       * Check 2: Wait for identifier value types to be inferred.
       *   token.get_value_type() is semantic type (".unknown", ".int", "x[]", etc.)
       *   Only identifiers require inference; literals are already typed.
       * ----------------------------------------------------------------------
       */
        do i = 2 to tokens.0
           if rule[i] = "" then iterate
           ti = tokens[i]
           t_lex = strip(ti.get_type())
           if t_lex = "identifier" then do
              _status = "PENDING"
              if ti.get_value_type() = ".unknown" then return _status
           end
        end
      /* ---------------------------------------------------------------------
       * Check 3: Semantic constraints.
       *   Parameters must be an array/stem type (contains '[') if defined in rule set.
       *   (Fix: check bracket in VALUE type, not lexical type.)
       * ----------------------------------------------------------------------
       */
      do i = 2 to tokens.0
         if rule[i] = "" then iterate
         if pos("[",rule[i])=0 then iterate
         ti = tokens[i]
         t_val = strip(ti.get_value_type())
         if pos("[", t_val)=0 then do /* condition failed */
            _status = "ERROR"
            _error_token = i
            _error_message = i". parameter '"ti.get_text()"' must be a stem/array"
            return _status
         end
      end
      /* --------------------------------------------------------------------
       * Step 4: Build replacement.
       *   IMPORTANT: only return REPLACE when replacement is non-empty.
       * ---------------------------------------------------------------------
       */
        _replacement = "__rc=arraysort({2},{3},{4},{5});"
        _status = "REPLACE"
        call log "replacement=" _replacement
        return _status

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

/* Helper: test if token type is allowed by the rule string */
_type_allowed: procedure = .int
   arg t_type = .string, allowed = .string
   allowed=translate(allowed,,'/\')
   do j = 1 to words(allowed)

      if t_type     = word(allowed, j) then return 1
      if t_type"[]" = word(allowed, j) then return 1  /* check if an array is requested */
   end
return 0

/***** Logging ***************************************************************
 * Keep logging side-effects cheap and safe:
 * - Prefer appending (lineout does)
 * - Consider gating via an env var so normal builds aren’t slowed down
 * ****************************************************************************
 */
log: procedure = .int
  arg logtxt = .string
  return 0
return lineout("c:\temp\pluginlog.txt", time()" "logtxt)