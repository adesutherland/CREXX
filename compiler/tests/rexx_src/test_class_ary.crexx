options levelb
import rxfnsb

s = .myclass()
call s.set("key1", "val1")
say "val=" s.get("key1")

return 0

myclass: class
  buckets = .int[]
  keys = .string[]
  values = .string[]
  next = .int[]
  count = .int

  *: factory
    t_buckets = .int[256]
    buckets = t_buckets
    
    t_keys = .string[1024]
    keys = t_keys
    
    t_values = .string[1024]
    values = t_values
    
    t_next = .int[1024]
    next = t_next
    
    count = 0
    return

  hash: method = .int
    arg key = .string
    h = 0
    len = length(key)
    if len = 0 then return 1
    
    do i = 1 to len
      char = substr(key, i, 1)
      val = c2d(char)
      h = h * 31 + val
    end
    
    q = h % 256
    rem = h - q * 256
    if rem < 0 then rem = -rem
    return rem + 1

  set: method = .void
    arg key = .string, value = .string
    
    t_buckets = buckets
    t_keys = keys
    t_values = values
    t_next = next
    
    b = hash(key)
    idx = t_buckets[b]
    last_idx = 0
    
    do while idx > 0
      if t_keys[idx] = key then do
        t_values[idx] = value
        values = t_values
        return
      end
      last_idx = idx
      idx = t_next[idx]
    end
    
    count = count + 1
    
    t_keys[count] = key
    t_values[count] = value
    t_next[count] = 0
    
    if last_idx > 0 then do
      t_next[last_idx] = count
    end
    else do
      t_buckets[b] = count
    end
    
    keys = t_keys
    values = t_values
    next = t_next
    buckets = t_buckets
    
    return

  get: method = .string
    arg key = .string
    
    t_buckets = buckets
    t_keys = keys
    t_values = values
    t_next = next
    
    b = hash(key)
    idx = t_buckets[b]
    
    do while idx > 0
      if t_keys[idx] = key then return t_values[idx]
      idx = t_next[idx]
    end
    
    return ""
