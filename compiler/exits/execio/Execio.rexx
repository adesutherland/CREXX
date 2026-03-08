options levelb
namespace rxcpexits expose execioexit
import rxcp
import rxfnsb

execioexit: class
    _node_id = .int with register.1
    _replacement = .string with register.2
    _error_token = .int with register.3
    _error_message = .string with register.4
    _status = .string with register.5

    *: factory
        arg nid = .int
        _node_id = nid
        _replacement = ""
        _error_token = 0
        _error_message = ""
        _status = "EMPTY"
        return

    get_primary_keyword: method = .string
        return "execio"

    get_additional_keywords: method = .string
        return ""

    process: method = .string
        arg tokens = .token[]

    /* reset per-call state */
    _replacement = ""
    _error_token = 0
    _error_message = ""
    _status = "EMPTY"

    /* minimal: EXECIO x y z => tokens.0 >= 4 (keyword + 3 parts) */
    if tokens.0 < 4 then do
       _status = "REJECT"
       return _status
    end
  /* ---------------------------------------------------------------------
   * Check 1: lexical sanity (broad)
   *  We accept only a conservative set for now:
   *    identifier, int_literal, string_literal, operator, parentheses
   * ---------------------------------------------------------------------
   */
    allowed = "identifier int_literal string_literal operator bracket comma other"
    do i = 2 to tokens.0
       ti = tokens[i]
       t_type = strip(ti.get_type())
       if pos(t_type, allowed) > 0 then iterate
       _status="ERROR"; _error_token=i
       _error_message="Unsupported token type in EXECIO: <"t_type"> text=<"ti.get_text()">"
       return _status
    end
   /* --------------------------------------------------------------------
    * Check 2: wait until identifiers are available
    * --------------------------------------------------------------------
    */
    do i = 2 to tokens.0
        ti = tokens[i]
        if strip(ti.get_type())="identifier" & ti.get_value_type() = ".unknown" then do
            _status = "PENDING"
            return _status
        end
    end
   /* ------------------------------------------------------------
    *  Find op keyword position (top-level, outside option block)
    * ------------------------------------------------------------
    */
    ops = "DISKR DISKW DISKA READ WRITE APPEND DISKRU DISKSU"
    opPos = 0
    parenDepth = 0
    do i = 2 to tokens.0
       txt = tokens[i].get_text()
       if txt="(" then parenDepth = parenDepth+1
       else if txt=")" then parenDepth = parenDepth-1
       if parenDepth \= 0 then iterate
       if pos(translate(txt), ops) > 0 then do
          opPos = i
          leave
       end
    end
    if opPos = 0 then do
        _status="ERROR"
        _error_token=2
        _error_message="EXECIO: missing operation (DISKR/DISKW/...)"
        return _status
    end
   /* file name is next after DISKx parameter */
    if opPos+1 > tokens.0 then do
        _status="ERROR"
        _error_token=opPos
        _error_message="EXECIO: missing ddname/filename after operation"
        return _status
    end
   /* ------------------------------------------------------------
    * Join count-expr span: tokens[2..opPos-1]
    *  (spanning-ready; may be 1 token or expression like recs+10)
    * ------------------------------------------------------------
    */
    countExpr = ""
    do i = 2 to opPos-1
        countExpr=countExpr||tokens[i].get_text()
    end
    if strip(countExpr) = "" then do
        _status="ERROR"
        _error_token=2
        _error_message="EXECIO: missing record count"
        return _status
    end
 /* quoting rule: '*' must be quoted; numeric expr stays as-is */
    if strip(countExpr) = "*" then countEmit = "'"countExpr"'"
    else countEmit = countExpr
 /* operation always literal => quoted */
    opEmit = tokens[opPos].get_text()                /* to do: there seems to be a spurious whitespace/char which can be dropped by strio function */
 /* ddname/fname in command form is literal => quoted unless already string_literal */
    fnameTok = tokens[opPos+1]
    if strip(fnameTok.get_type()) = "string_literal" then fnameEmit = fnameTok.get_text()
    else fnameEmit = "'"fnameTok.get_text()"'"   /* force literal */
   /* ------------------------------------------------------------
    * Optional option block: parse STEM <identifier> and flags
    * ------------------------------------------------------------
    */
    stemPresent = 0
    stemEmit = ""
    p = opPos + 2
    if p <= tokens.0 & tokens[p].get_text() = "(" then do
       p =p+1
       do while p <= tokens.0
          txt = tokens[p].get_text()
          up  = translate(txt)
          if txt = ")" then do
              p =p+1
              leave
          end
          if up = "STEM" then do
             p =p+1
             if p > tokens.0 then do
                _status="ERROR"; _error_token=p-1
                _error_message="EXECIO: STEM requires a value"
                return _status
             end
             vtok = tokens[p]
             if strip(vtok.get_type()) \= "identifier" then do
                _status="ERROR"; _error_token=p
                _error_message="EXECIO: STEM value "vtok.get_text()" mandatory stem (no ending dot)"
                return _status
             end
             stemPresent = 1
             stemEmit = vtok.get_text()
             p =p+1
             iterate
          end
         /* accept FINIS etc (ignored for now) */
          if up = "FINIS" | up = "FIFO" | up = "LIFO" | up = "OPEN" then do
             p =p+1
             iterate
          end
          /* unknown option => error (or make permissive later) */
           _status="ERROR"; _error_token=p
           _error_message="EXECIO: unknown option '"txt"'"
           return _status
        end
    end
   /* ------------------------------------------------------------
    * Emit canonical call
    * ------------------------------------------------------------
    */
    opEmit=upper(strip(opEmit))
 /*  ++++ defined variables don't survive compiler exit end
    if opEmit='DISKR' then _replacement = stemEmit"=.string[];"
    else _replacement=''
  */
    _replacement=''
    _replacement = _replacement||"__rc=_execio("countEmit",'"opEmit"',"fnameEmit
    if stemPresent then _replacement = _replacement||", "stemEmit
    _replacement = _replacement||")"    /* close execio function call */
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
  return 0
return lineout("c:\temp\pluginlog.txt", time()" "logtxt)