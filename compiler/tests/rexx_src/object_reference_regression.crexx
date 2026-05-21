options levelb

main: procedure = .int
  payload = .LargePayload("seed")

  say "factory-summary=" || payload.summary()
  say "factory-check=" || payload.check()

  call replacePayloadRef(payload, "rebound")
  say "ref-rebind=" || payload.summary()

  call mutatePayloadRef(payload, "mutated")
  say "ref-mutate=" || payload.summary()

  local_summary = replacePayloadLocal(payload, "local")
  say "local-replace=" || local_summary
  say "local-after=" || payload.summary()

  return 0

replacePayloadRef: procedure = .void
  arg expose item = .LargePayload, label = .string
  item = .LargePayload(label)
  return

mutatePayloadRef: procedure = .void
  arg expose item = .LargePayload, label = .string
  call item.rename(label)
  return

replacePayloadLocal: procedure = .string
  arg item = .LargePayload, label = .string
  item = .LargePayload(label)
  return item.summary()

SmallPayload: class
  name = .string

  *: factory
    arg label = .string
    name = label
    return

  getName: method = .string
    return name

  setName: method = .void
    arg label = .string
    name = label
    return

LargePayload: class
  name = .string
  values = .string[]
  child = .SmallPayload
  marker1 = .string
  marker2 = .string
  marker3 = .string
  marker4 = .string
  marker5 = .string
  marker6 = .string
  marker7 = .string
  marker8 = .string

  *: factory
    arg label = .string
    t_values = .string[64]
    name = label
    do i = 1 to 40
      t_values[i] = label || ":" || i
    end
    values = t_values
    child = .SmallPayload(label || ":child")
    marker1 = label || ":m1"
    marker2 = label || ":m2"
    marker3 = label || ":m3"
    marker4 = label || ":m4"
    marker5 = label || ":m5"
    marker6 = label || ":m6"
    marker7 = label || ":m7"
    marker8 = label || ":m8"
    return

  rename: method = .void
    arg label = .string
    t_values = values
    name = label
    t_values[1] = label || ":1"
    t_values[40] = label || ":40"
    values = t_values
    call child.setName(label || ":child")
    marker8 = label || ":m8"
    return

  summary: method = .string
    child_name = child.getName()
    return name || "|" || values[1] || "|" || values[40] || "|" || child_name || "|" || marker8

  check: method = .string
    if values[17] \= name || ":17" then return "bad-17"
    if marker1 \= name || ":m1" then return "bad-m1"
    if marker8 \= name || ":m8" then return "bad-m8"
    return "ok"
