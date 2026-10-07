options levelc
arg first, second, third
say 'leaf-args=' || arg() || '|' || arg(1,'E') || '|' || arg(2,'E') || '|' || arg(3,'E')
say 'leaf-data=' || c2x(first) || '|' || length(first) || '|' || length(third) || '|' || third
say 'leaf-pool=' || symbol('SHARED')
say 'leaf-source=' || sourceline(2)
shared = 'leaf'
if arg() = 0 then return
return first
