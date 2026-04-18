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
     else if isParseKeywordToken(ti, "VALUE") | isParseKeywordToken(ti, "VAR") then do
        parmtype = utext
        call compile_plan.add_keyword(start_of_template, text, "parse_source", "parse")
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

  if parmtype\="" & source_found=0 then do
     call compile_plan.set_error(1, "PARSE " || parmtype || " requires a source expression", "MISSING_ARGUMENTS")
     return compile_plan
  end

   if parmtype="" then do
      start_of_template=2
      if uplow\="" then start_of_template=start_of_template+1
      if wantlog>0 then start_of_template=start_of_template+1
      if wanttrace>0 then start_of_template=start_of_template+1
      if _wanttrim>0 then start_of_template=start_of_template+1
      if _into\="" then start_of_template=start_of_template+2
  end

  if start_of_template > tokens.0 then do
     call compile_plan.set_error(1, "PARSE requires arguments", "MISSING_ARGUMENTS")
     return compile_plan
  end

  if wanttrace>0 then wantlog=9
  out = 0
  _template=''
  pkind = .int[]
  ptext = .string[]
  template_end = tokens.0

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

  i = start_of_template + 1
  haswith=0
  prevkind = 0

  do while i <= template_end
     ti   = tokens[i]
     type = strip(ti.get_type())
     text = strip(ti.get_text())

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
  tj = tokens[ti]
  _replacement=''
  parse_string = strip(tj.get_text())
  if _into='' then _into='_parseResult'
  if uplow='UPPER' then _replacement = _replacement"; _source=upper("parse_string")"
  else if uplow='LOWER' then _replacement = _replacement"; _source=lower("parse_string")"
  else _replacement = _replacement"; _source="parse_string
  _replacement = _replacement || '; ' || _into || '=parseExec(_source,' || quote_rexx_string(plan) || ',' || quote_rexx_string(_template) || ',' || wantlog || ')'
  return compile_plan

process: method = .exitresult
  arg tokens = .token[]

    result = .exitresult("EMPTY")
    if tokens.0 < 3 then do
       call result.set_status("REJECT")
       return result
    end

    allowed = "identifier exit_keyword keyword int_literal string_literal operator comma other bracket"
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

quote_rexx_string: procedure = .string
  arg raw = .string

  quoted = "'"
  do i = 1 to length(raw)
     ch = substr(raw, i, 1)
     if ch = "'" then quoted = quoted || "''"
     else quoted = quoted || ch
  end
  return quoted || "'"

isVar: procedure=.int
  arg varname=.string
  vlen=length(varname)
  do i = 1 to vlen
     c = substr(varname, i, 1)
     if pos(c, 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.') = 0 then return 0
  end
return 1

isParseIdentifierToken: procedure = .int
  arg tok = .token
  if strip(tok.get_type()) = "identifier" then return 1
  return 0

isParseKeywordToken: procedure = .int
  arg tok = .token, keyword = .string
  type = strip(tok.get_type())
  text = upper(strip(tok.get_text()))
  if text \= upper(keyword) then return 0
  if type = "identifier" | type = "exit_keyword" | type = "keyword" then return 1
  return 0
