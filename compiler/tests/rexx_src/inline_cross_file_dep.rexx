options levelb
namespace inline_cross_file_dep expose inc classify scoped

inc: procedure = .int
  arg value = .int
  return value + 1

classify: procedure = .int
  arg value = .int
  if value < 0 then return -1
  if value > 0 then return 1
  return 0

scoped: procedure = .int
  arg value = .int
  if value > 0 then do
    tmp = .int
    tmp = value * 10
    return tmp
  end
  return value
