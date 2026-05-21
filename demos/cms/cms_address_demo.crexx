options levelb
namespace cms_address_demo expose main
import rxfnsb

main: procedure = .int
  buffer_out = .string[]
  list_out = .string[]
  msg_out = .string[]
  state_out = .string[]
  type_out = .string[]
  userid_out = .string[]
  args = .string[]
  env_instance = .addressenvironment
  env_functions = .addressfunctionenvironment
  response = .addressfunctionresponse
  note = .string

  say "cREXX CMS ADDRESS demo"

  address cms

  "CP QUERY USERID" output userid_out
  call show_lines("CP QUERY USERID", userid_out)

  "QUERY MSG" output msg_out
  call show_lines("QUERY MSG", msg_out)

  "SET MSG OFF"
  call show_value("MSG_MODE()", addresscall("cms", "msg_mode"))

  "LISTFILE * * A" output list_out
  call show_lines("LISTFILE * * A", list_out)

  args = .string[]
  call show_value("FILES()", addresscall("cms", "files"))

  "STATE PROFILE EXEC A" output state_out
  call show_lines("STATE PROFILE EXEC A", state_out)

  "TYPE README EXEC" output type_out
  call show_lines("TYPE README EXEC", type_out)

  "MAKEBUF" output buffer_out
  call show_lines("MAKEBUF", buffer_out)

  note = "Rexx CMS host variable anchor"
  "NOTE :note"
  call show_value("LAST_NOTE()", addresscall("cms", "last_note"))

  env_instance = addressenv("cms")
  say ""
  say "ADDRESS instance => name=" || env_instance.environment_name() || " id=" || env_instance.environment_id()

  args = .string[]
  args[1] = "profile exec a"
  call show_value("FILEINFO(profile exec a)", addresscall("cms", "fileinfo", "profile exec a"))

  args = .string[]
  args[1] = "hello cms friends"
  args[2] = "upper"
  env_functions = addressenv("cms") as .addressfunctionenvironment
  response = env_functions.invoke(.addressfunctionrequest("cms", "pipe", args, .standardaddresssandbox(), ""))
  call show_function("PIPE(text, upper) via object", response)

  args = .string[]
  args[1] = "a small virtual reader queue"
  call show_value("WORDS(text)", addresscall("cms", "words", "a small virtual reader queue"))

  return 0

show_lines: procedure = .void
  arg label = .string, lines = .string[]
  say ""
  say "== " || label
  if lines.0 < 1 then do
    say "(no output)"
    return
  end
  do i = 1 to lines.0
    say lines[i]
  end
  return

show_function: procedure = .void
  arg label = .string, response = .addressfunctionresponse
  say label || " => rc=" || response.get_rc() || " result=" || response.get_result()
  return

show_value: procedure = .void
  arg label = .string, value = .string
  say label || " => " || value
  return
