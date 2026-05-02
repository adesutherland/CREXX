options levelb
namespace kv_address_demo expose main
import rxfnsb

main: procedure = .int
  count_out = .string[]
  dump_out = .string[]
  mode_out = .string[]
  user_out = .string[]
  env_instance = .addressinstance
  fetched = .string
  key = .string
  value = .string

  say "cREXX native KV ADDRESS demo"

  address kv

  key = "user"
  value = "Adrian"
  "PUT :key :value"
  fetched = ""
  "GET :key INTO ${fetched}"
  call show_value("GET :key INTO ${fetched}", fetched)

  key = "project"
  value = "cREXX native ADDRESS"
  "PUT ${key} ${value}"

  "GET user" output user_out
  call show_lines("GET user", user_out)

  say "ID() => " || _address_call("kv", "id")
  say "GET(user) => " || _address_call("kv", "get", "user")
  say "EXISTS(project) => " || _address_call("kv", "exists", "project")
  say "COUNT() => " || _address_call("kv", "count")

  say "PUT(mode, function-call) => " || _address_call("kv", "put", "mode", "function-call")
  "GET mode" output mode_out
  call show_lines("GET mode", mode_out)

  "COUNT" output count_out
  call show_lines("COUNT command", count_out)

  "DUMP" output dump_out
  call show_lines("DUMP", dump_out)

  env_instance = _address_environment("kv") as .addressinstance
  say ""
  say "ADDRESS instance => name=" || env_instance.environment_name() || " id=" || env_instance.environment_id()

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

show_value: procedure = .void
  arg label = .string, value = .string
  say label || " => " || value
  return
