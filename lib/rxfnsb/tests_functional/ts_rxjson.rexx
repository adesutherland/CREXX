options levelb
import rxjson

failures = 0

failures = failures + check_int("valid object", jsonvalid('{"ok":true,"n":12}'), 1)
failures = failures + check_int("invalid object", jsonvalid('{"ok":true,}'), 0)
failures = failures + check_str("root type", jsontype('{"ok":true}', ''), "object")
failures = failures + check_str("missing type", jsontype('{"ok":true}', 'nope'), "missing")
failures = failures + check_str("bool type", jsontype('{"ok":true}', 'ok'), "boolean")
failures = failures + check_str("bool get", jsonget('{"ok":true}', 'ok'), "true")
failures = failures + check_str("number get", jsonget('{"n":12.5}', 'n'), "12.5")
failures = failures + check_str("quote string", jsonquote('hello "json"'), '"hello \"json\""')
failures = failures + check_str("unquote string", jsonunquote('"hello \"json\""'), 'hello "json"')

response = '{"choices":[{"message":{"content":"Hello from JSON"},"finish_reason":"stop"}],"usage":{"total_tokens":42}}'
failures = failures + check_str("llm content path", jsonget(response, "choices.1.message.content"), "Hello from JSON")
failures = failures + check_str("llm finish path", jsonget(response, "choices[1].finish_reason"), "stop")
failures = failures + check_int("array count", jsoncount(response, "choices"), 1)
failures = failures + check_int("object count", jsoncount(response, "usage"), 1)

members = .string[]
failures = failures + check_int("root members count", jsonmembers(response, "", members), 2)
failures = failures + check_str("root member one", members[1], "choices")
failures = failures + check_str("root member two", members[2], "usage")

values = .string[]
values[1] = jsonquote("system")
values[2] = jsonquote("hello")
roles = jsonarray(values)
failures = failures + check_str("array build", roles, '["system","hello"]')
failures = failures + check_str("array path numeric", jsonget(roles, "2"), "hello")

msg_keys = .string[]
msg_values = .string[]
msg_keys[1] = "role"
msg_values[1] = jsonquote("user")
msg_keys[2] = "content"
msg_values[2] = jsonquote("Say hi")
message = jsonobject(msg_keys, msg_values)
failures = failures + check_int("message valid", jsonvalid(message), 1)
failures = failures + check_str("message content", jsonget(message, "content"), "Say hi")

body_keys = .string[]
body_values = .string[]
messages = .string[]
messages[1] = message
body_keys[1] = "model"
body_values[1] = jsonquote("demo-model")
body_keys[2] = "messages"
body_values[2] = jsonarray(messages)
body = jsonobject(body_keys, body_values)
failures = failures + check_int("llm body valid", jsonvalid(body), 1)
failures = failures + check_str("llm body model", jsonget(body, "model"), "demo-model")
failures = failures + check_str("llm body message", jsonget(body, "messages.1.content"), "Say hi")

if failures <> 0 then do
  say "rxjson failures:" failures
  exit 1
end

exit 0

check_int: procedure = .int
  arg label = .string, actual = .int, expected = .int
  if actual <> expected then do
    say label": expected" expected "got" actual
    return 1
  end
  return 0

check_str: procedure = .int
  arg label = .string, actual = .string, expected = .string
  if actual <> expected then do
    say label": expected ["expected"] got ["actual"]"
    return 1
  end
  return 0
