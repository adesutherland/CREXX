options levelb
namespace rxcpexits expose sortexit2
import rxcp
import rxfnsb


sortexit2: class
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
        return

    get_primary_keyword: method = .string
        return "sortx"

    get_additional_keywords: method = .string
        return ""

    process: method = .string
        arg tokens = .token[]

  /* reset per-call state */
    _replacement = ""
    _error_token = 0
    _error_message = ""
    _status = "EMPTY"
    /* ---------------------------------------------------------------------
     * Check 0: minimal count for sortx <stem> <expr> <order>
     * --------------------------------------------------------------------
     */
    if tokens[0] < 4 then do
       _status = "REJECT"
       return _status
    end

  /* ---------------------------------------------------------------------
   * Check 1: lexical sanity (broad)
   *  We accept only a conservative set for now:
   *    identifier, int_literal, string_literal, operator, parentheses
   *  This is intentionally permissive because spanning comes later.
   * ---------------------------------------------------------------------
   */
   do i = 2 to tokens[0]
      ti = tokens[i]
      t_type = strip(ti.get_type())
      t_text = ti.get_text()
      if t_type = "identifier"     then iterate
      if t_type = "int_literal"    then iterate
      if t_type = "string_literal" then iterate
      if t_type = "operator"       then iterate
      if t_type = "bracket"        then iterate
      if t_type = "comma"          then iterate
      if t_type = "other"          then iterate
   /* throw error and terminate function */
      _status = "ERROR"
      _error_token = i
      _error_message = "Unsupported token type in sortx: <"t_type"> text=<"t_text">"
      return _status
   end
  /* --------------------------------------------------------------------
   * Check 2: wait until identifiers are available
   * --------------------------------------------------------------------
   */
   do i = 2 to tokens[0]
      ti = tokens[i]
      if strip(ti.get_type()) = "identifier" & ti.get_value_type() = ".unknown" then do
         _status = "PENDING"
         return _status
      end
   end
  /* ---------------------------------------------------------------------
   * Spanning: split tokens[2..tokens[0]] into comma-separated ranges.
   * Comma is a separator only at top level (parenDepth == 0).
   * --------------------------------------------------------------------- 
   */
   ranges = .string[]
   split_indx= 2
   parenDepth = 0
   _status = "ERROR"        /* assume the worst */
   do i = 2 to tokens[0]
      ti  = tokens[i]
      txt = ti.get_text()
   /* Track parentheses */
      if txt = "(" then parenDepth = parenDepth + 1
      else if txt = ")" then do
         parenDepth = parenDepth - 1
         if parenDepth < 0 then do
            _error_token = i
            _error_message = "Unbalanced ')'"
            return _status
         end
      end
   /* Split on comma at top level */
      if txt = "," & parenDepth = 0 then do
         if i = split_indx then do
            _error_token = i
            _error_message = "Empty parameter before comma"
            return _status
         end
         ranges[ranges[0] + 1] = split_indx" "i-1
         split_indx= i + 1
      end
   end
   if parenDepth \= 0 then do
      _error_token = 1
      _error_message = "Unbalanced parentheses in parameter list"
      return _status
   end
   if split_indx> tokens[0] then do
      _error_token = 1
      _error_message = "Empty final parameter"
      return _status
   end
   ranges[ranges[0] + 1] = split_indx" "tokens[0]
  /* ---------------------------------------------------------------------
   *  Join: convert each range into an argument expression string.
   * ---------------------------------------------------------------------
   */
   args = .string[]
   do j = 1 to ranges[0]
      split_start = word(ranges[j], 1)
      split_end  = word(ranges[j], 2)
      joined_parm = ""
      do i = split_start to split_end
         ti = tokens[i]
         t_type = strip(ti.get_type())
         t_text = ti.get_text()
      /* if whitespace tokens exist in the future, skip them */
      /* if t_type = "whitespace" then iterate */
         joined_parm = joined_parm || t_text
      end
      if strip(joined_parm) = "" then do
         _status = "ERROR"
         _error_token = start
         _error_message = "Empty expression"
         return _status
      end
      args[args[0] + 1] = joined_parm
  end
  /* ---------------------------------------------------------------------
   * Check 3: Semantic constraints.
   *   Parameters must be an array/stem type (contains '[') if defined in rule set.
   *   (Fix: check bracket in VALUE type, not lexical type.)
   * ----------------------------------------------------------------------
   */
    k=2                                 /* in the moment we check only first parameter of sortx */
    ti = tokens[k]                      /* address to first parms */
    t_val = strip(ti.get_value_type())
    if pos("[", t_val)=0 then do        /* condition failed */
       _status = "ERROR"
       _error_token = k
       _error_message = k". parameter <"ti.get_text()"> must be a stem/array"
       return _status
    end
  /* ---------------------------------------------------------------------
   *  Build replacement: arraysort(arg1,arg2,arg3,...)
   *  --------------------------------------------------------------------
   */
   parms = args[1]
   do j = 2 to args[0]
      parms = parms","args[j]
   end
   _replacement= "__rc=arraysort("parms")"
   call log _replacement
   _status = "REPLACE"
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

/***** Logging ***************************************************************
 * Keep logging side-effects cheap and safe:
 * - Prefer appending (linejoined_parm does)
 * - Consider gating via an env var so normal builds aren’t slowed down
 * ****************************************************************************
 */
log: procedure = .int
  arg logtxt = .string
return lineout("c:\temp\pluginlog.txt", time()" "logtxt)