options levelb

main: procedure
  input_lines = .string[]
  output_lines = .string[]
  error_lines = .string[]

  input_lines[1] = "b"
  input_lines[2] = "a"

  address path "sort" input input_lines output output_lines error error_lines

  say output_lines[1]
  say output_lines[2]
  return
