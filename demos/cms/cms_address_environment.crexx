/* CMS-style ADDRESS environment demonstrator.
 *
 * This is intentionally a Rexx provider, not a native plugin, so it shows the
 * new environment instance and function-dispatch path end to end in Rexx.
 */
options levelb
namespace _rxsysb expose cmsaddressenvironment
import _rxsysb
import rxfnsb

cmsaddressenvironment: class implements .addressenvironment .addressinstance .addressfunctionenvironment
  _environment_id = .string
  _environment_name = .string

  *: match
    arg env_name = .string
    if _address_normalize_environment_name(env_name) = "CMS" then return 100
    return 0

  *: factory
    arg env_name = .string
    _environment_name = _address_normalize_environment_name(env_name)
    _environment_id = "rexx:" || _environment_name
    call cms_ensure_runtime
    return

  bind_environment: method = .void
    arg env_name = .string, instance_id = .string
    _environment_name = _address_normalize_environment_name(env_name)
    _environment_id = instance_id
    if _environment_id = "" then _environment_id = "rexx:" || _environment_name
    return

  environment_name: method = .string
    return _environment_name

  environment_id: method = .string
    return _environment_id

  execute: method = .addressresponse
    arg request = .addressrequest

    normalized = .string
    original = .string
    prefix8 = .string
    prefix7 = .string
    prefix6 = .string
    prefix5 = .string
    prefix4 = .string
    response = .addressresponse
    binding = .addressbinding
    sandbox_value = .string
    binding_kind = .string
    filespec = .string

    original = strip(request.get_command())
    normalized = _address_normalize_command(original)
    prefix8 = _address_first_chars(normalized, 8)
    prefix7 = _address_first_chars(normalized, 7)
    prefix6 = _address_first_chars(normalized, 6)
    prefix5 = _address_first_chars(normalized, 5)
    prefix4 = _address_first_chars(normalized, 4)

    if normalized = "CP SET MSG ON" | normalized = "SET MSG ON" then do
      call cms_set_msg_mode("ON")
      return .addressresponse(0)
    end

    if normalized = "CP SET MSG OFF" | normalized = "SET MSG OFF" then do
      call cms_set_msg_mode("OFF")
      return .addressresponse(0)
    end

    if normalized = "QUERY MSG" | normalized = "CP QUERY MSG" then do
      return cms_emit1(request, "MSG = " || cms_get_msg_mode())
    end

    if normalized = "QUERY ADDRESS" then do
      return cms_emit2(request, "CMS ADDRESS environment is active", "Use ID() to inspect the bound instance")
    end

    if normalized = "CP QUERY USERID" then return cms_emit1(request, "CMSUSER")
    if normalized = "CP QUERY TIME" then return _address_execute_system_command(request, "date '+TIME IS %H:%M:%S %Z ON %Y-%m-%d'")
    if normalized = "QUERY DISK" | normalized = "QUERY DISKS" then do
      return cms_emit3(request, "LABEL  VDEV M  STAT   CYL TYPE BLKSIZE FILES", "CMS191 191  A  R/W    120 3380 4096    42", "CMS192 192  B  R/O     40 3380 4096     7")
    end

    if prefix8 = "LISTFILE" then do
      filespec = subword(normalized, 2)
      if filespec = "" | word(filespec, 1) = "*" then do
        return cms_emit1(request, "DEMO EXEC A1")
      end
      if cms_file_exists(filespec) = 1 then return cms_emit1(request, cms_file_listing(filespec))
      return cms_missing_file_response(filespec)
    end

    if prefix5 = "STATE" then do
      filespec = subword(normalized, 2)
      if cms_file_exists(filespec) = 1 then return cms_emit1(request, cms_file_listing(filespec))
      return cms_missing_file_response(filespec)
    end

    if prefix4 = "TYPE" then do
      filespec = subword(normalized, 2)
      if cms_normalize_filespec(filespec) = "README EXEC A" then do
        return cms_emit1(request, "CMS TYPE DEMO")
      end
      if cms_normalize_filespec(filespec) = "PROFILE EXEC A" then do
        return cms_emit4(request, "/* PROFILE EXEC */", "address cms 'QUERY MSG'", "address cms 'LISTFILE * * A'", "say 'READY;'")
      end
      if cms_normalize_filespec(filespec) = "PIPELINE EXEC A" then do
        return cms_emit4(request, "/* PIPELINE EXEC */", "address cms 'MAKEBUF'", "address cms 'NOTE PIPELINE DEMO RAN'", "address cms 'QUERY BUFFER'")
      end
      return cms_missing_file_response(filespec)
    end

    if normalized = "MAKEBUF" then do
      return cms_emit1(request, "BUFFER " || cms_make_buffer() || " CREATED")
    end

    if normalized = "DROPBUF" then do
      return cms_emit1(request, "BUFFER DEPTH " || cms_drop_buffer())
    end

    if normalized = "QUERY BUFFER" then do
      return cms_emit1(request, "BUFFER DEPTH " || cms_buffer_depth_text())
    end

    if prefix4 = "NOTE" then do
      call cms_set_last_note(cms_resolve_host_text(request, subword(original, 2)))
      return .addressresponse(0)
    end

    if prefix7 = "EXEC CMS" | prefix8 = "EXEC CMS " then do
      return cms_emit2(request, "DMSXEC001I EXEC request accepted by Rexx CMS demo", "READY")
    end

    if prefix6 = "PIPE U" then do
      return cms_emit1(request, upper(subword(original, 3)))
    end

    if normalized = "SANDBOX ROUNDTRIP" then do
      sandbox_value = request.get_sandbox_value("VALUE.3")
      call request.set_sandbox_value("result", sandbox_value || ":cms")
      call request.set_sandbox_value("value.4", "cms-four")
      return .addressresponse(0)
    end

    if normalized = "SANDBOX RESPONSE UPDATE" then do
      response = .addressresponse(0)
      call response.add_updated_binding("sandbox", "response", "response", "cms-response", "")
      return response
    end

    if normalized = "EXPOSE ARRAY" then do
      response = .addressresponse(0)
      do i = 1 to request.get_binding_count()
        binding = request.get_binding(i)
        binding_kind = _address_normalize_environment_name(binding.get_kind())
        if binding_kind = "STEM" & _address_normalize_environment_name(binding.get_internal_name()) = "ITEMS" then do
          if request.get_binding_stem_value(i, "1") = "cms-one" then do
            call request.set_binding_stem_value(i, "2", "cms-two-updated")
            call request.set_binding_stem_value(i, "3", "cms-three")
            call request.set_binding_stem_value(i, "0", "3")
          end
        end
      end
      return response
    end

    if normalized = "SET BUFFER UPDATED" then do
      response = .addressresponse(0)
      do i = 1 to request.get_binding_count()
        binding = request.get_binding(i)
        if _address_normalize_environment_name(binding.get_kind()) = "VAR" & _address_normalize_environment_name(binding.get_internal_name()) = "BUFFER" then do
          call response.add_updated_binding("var", binding.get_internal_name(), binding.get_external_alias(), "cms-updated", "")
        end
      end
      return response
    end

    return _address_unknown_command_response("CMS", request.get_command())

  invoke: method = .addressfunctionresponse
    arg request = .addressfunctionrequest

    function_name = .string
    filespec = .string
    response = .addressfunctionresponse

    function_name = _address_normalize_environment_name(request.get_function_name())
    if function_name = "ID" then return .addressfunctionresponse(0, _environment_id)
    if function_name = "NAME" then return .addressfunctionresponse(0, _environment_name)
    if function_name = "MSG_MODE" then return .addressfunctionresponse(0, cms_get_msg_mode())
    if function_name = "BUFFER_DEPTH" then return .addressfunctionresponse(0, cms_buffer_depth_text())
    if function_name = "LAST_NOTE" then return .addressfunctionresponse(0, cms_get_last_note())
    if function_name = "ECHO" then return .addressfunctionresponse(0, request.get_argument(1) || ":cms:" || _environment_id)
    if function_name = "UPPER" then return .addressfunctionresponse(0, upper(request.get_argument(1)))
    if function_name = "WORDS" then return .addressfunctionresponse(0, "" || cms_count_words(request.get_argument(1)))
    if function_name = "DISK_FREE" then return .addressfunctionresponse(0, "A 191 37% USED 63% FREE")
    if function_name = "FILES" then return .addressfunctionresponse(0, "DEMO EXEC A; PROFILE EXEC A; README EXEC A; PIPELINE EXEC A")

    if function_name = "STATE" then do
      filespec = request.get_argument(1)
      if cms_file_exists(filespec) = 1 then return .addressfunctionresponse(0, "READY")
      return .addressfunctionresponse(28, "MISSING", "FAILURE")
    end

    if function_name = "FILEINFO" then do
      filespec = request.get_argument(1)
      if cms_file_exists(filespec) = 1 then return .addressfunctionresponse(0, cms_file_listing(filespec))
      return .addressfunctionresponse(28, "MISSING", "FAILURE")
    end

    if function_name = "PIPE" then do
      if _address_normalize_environment_name(request.get_argument(2)) = "UPPER" then return .addressfunctionresponse(0, upper(request.get_argument(1)))
      if _address_normalize_environment_name(request.get_argument(2)) = "WORDS" then return .addressfunctionresponse(0, "" || cms_count_words(request.get_argument(1)))
      return .addressfunctionresponse(0, request.get_argument(1))
    end

    response = .addressfunctionresponse(-3, "")
    call response.set_condition_name("FAILURE")
    call response.add_diagnostic("Unknown CMS function " || request.get_function_name())
    return response

cms_emit1: procedure = .addressresponse
  arg request = .addressrequest, line1 = .string
  return _address_execute_system_command(request, "echo " || line1)

cms_emit2: procedure = .addressresponse
  arg request = .addressrequest, line1 = .string, line2 = .string
  return _address_execute_system_command(request, "echo " || line1 || " -- " || line2)

cms_emit3: procedure = .addressresponse
  arg request = .addressrequest, line1 = .string, line2 = .string, line3 = .string
  return _address_execute_system_command(request, "echo " || line1 || " -- " || line2 || " -- " || line3)

cms_emit4: procedure = .addressresponse
  arg request = .addressrequest, line1 = .string, line2 = .string, line3 = .string, line4 = .string
  return _address_execute_system_command(request, "echo " || line1 || " -- " || line2 || " -- " || line3 || " -- " || line4)

cms_resolve_host_text: procedure = .string
  arg request = .addressrequest, text = .string
  anchor_name = .string

  anchor_name = cms_host_anchor_name(strip(text))
  if anchor_name = "" then return text
  return request.get_binding_value(anchor_name)

cms_host_anchor_name: procedure = .string
  arg token = .string
  name = .string

  if length(token) > 1 & left(token, 1) = ":" then do
    name = substr(token, 2)
    if cms_host_anchor_name_ok(name) = 1 then return name
  end

  if length(token) > 3 & left(token, 2) = "${" & right(token, 1) = "}" then do
    name = substr(token, 3, length(token) - 3)
    if cms_host_anchor_name_ok(name) = 1 then return name
  end

  return ""

cms_host_anchor_name_ok: procedure = .int
  arg name = .string
  ch = .string

  if length(name) < 1 then return 0
  ch = substr(name, 1, 1)
  if cms_host_anchor_start(ch) \= 1 then return 0

  do i = 2 to length(name)
    if cms_host_anchor_part(substr(name, i, 1)) \= 1 then return 0
  end

  return 1

cms_host_anchor_start: procedure = .int
  arg ch = .string
  if ch = "_" then return 1
  if pos(upper(ch), "ABCDEFGHIJKLMNOPQRSTUVWXYZ") > 0 then return 1
  return 0

cms_host_anchor_part: procedure = .int
  arg ch = .string
  if cms_host_anchor_start(ch) = 1 then return 1
  if pos(ch, "0123456789") > 0 then return 1
  return 0

cms_ensure_runtime: procedure expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  if _cms_demo_runtime_ready = 1 then return
  _cms_demo_msg_mode = "ON"
  _cms_demo_buffer_depth = 0
  _cms_demo_last_note = ""
  _cms_demo_runtime_ready = 1
  return

cms_set_msg_mode: procedure = .void expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  arg mode = .string
  call cms_ensure_runtime
  _cms_demo_msg_mode = mode
  return

cms_get_msg_mode: procedure = .string expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  call cms_ensure_runtime
  return _cms_demo_msg_mode

cms_make_buffer: procedure = .string expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  call cms_ensure_runtime
  _cms_demo_buffer_depth = _cms_demo_buffer_depth + 1
  return "" || _cms_demo_buffer_depth

cms_drop_buffer: procedure = .string expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  call cms_ensure_runtime
  if _cms_demo_buffer_depth > 0 then _cms_demo_buffer_depth = _cms_demo_buffer_depth - 1
  return "" || _cms_demo_buffer_depth

cms_buffer_depth_text: procedure = .string expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  call cms_ensure_runtime
  return "" || _cms_demo_buffer_depth

cms_set_last_note: procedure = .void expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  arg note = .string
  call cms_ensure_runtime
  _cms_demo_last_note = note
  return

cms_get_last_note: procedure = .string expose _cms_demo_runtime_ready _cms_demo_msg_mode _cms_demo_buffer_depth _cms_demo_last_note
  call cms_ensure_runtime
  return _cms_demo_last_note

cms_missing_file_response: procedure = .addressresponse
  arg filespec = .string
  response = .addressresponse(28)
  call response.set_condition_name("FAILURE")
  call response.add_diagnostic("CMS file not found " || cms_normalize_filespec(filespec))
  return response

cms_normalize_filespec: procedure = .string
  arg filespec = .string
  normalized = .string
  filename = .string
  filetype = .string
  filemode = .string

  normalized = _address_normalize_command(filespec)
  filename = word(normalized, 1)
  filetype = word(normalized, 2)
  filemode = word(normalized, 3)
  if filename = "" then return ""
  if filetype = "" then filetype = "EXEC"
  if filemode = "" then filemode = "A"
  if length(filemode) > 1 then filemode = substr(filemode, 1, 1)
  return filename || " " || filetype || " " || filemode

cms_file_exists: procedure = .int
  arg filespec = .string
  normalized = .string
  normalized = cms_normalize_filespec(filespec)
  if normalized = "DEMO EXEC A" then return 1
  if normalized = "PROFILE EXEC A" then return 1
  if normalized = "README EXEC A" then return 1
  if normalized = "PIPELINE EXEC A" then return 1
  return 0

cms_file_listing: procedure = .string
  arg filespec = .string
  normalized = .string
  normalized = cms_normalize_filespec(filespec)
  if normalized = "DEMO EXEC A" then return "DEMO EXEC A1"
  if normalized = "PROFILE EXEC A" then return "PROFILE EXEC A1 V 80 12 1 2026-05-02 09:00:00"
  if normalized = "README EXEC A" then return "README EXEC A1 V 80 8 1 2026-05-02 09:01:00"
  if normalized = "PIPELINE EXEC A" then return "PIPELINE EXEC A1 V 80 15 1 2026-05-02 09:02:00"
  return ""

cms_count_words: procedure = .int
  arg text = .string
  count = .int
  count = 0
  do i = 1 to 64
    if word(text, i) = "" then return count
    count = count + 1
  end
  return count
